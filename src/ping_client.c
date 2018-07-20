//
//  ping_client.c
//  project3
//
//  Created by Adam on 2/26/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#include "ping_client.h"

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#include "stats.h"

#define BUFFER_SIZE 1000

pthread_t current_thread_t;

struct ping_client
{
	flags_handle flags;
};

//method to get port number
in_port_t getInPort(struct sockaddr * sa)
{
	if (sa->sa_family == AF_INET) { //IPv4
		return ntohs(((struct sockaddr_in *)sa)->sin_port);
	}
	return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
}

//method to get ip address and return it in 'ip'
void getIP(struct sockaddr * sa, char * ip)
{
	if (sa->sa_family == AF_INET) { //IPv4
		char ip4[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, (struct sockaddr_in *)sa, ip4, INET_ADDRSTRLEN);
		strcpy(ip, ip4);
		return;
	}
	char ip6[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, (struct sockaddr_in6 *)sa, ip6, INET6_ADDRSTRLEN);
	strcpy(ip, ip6);
	return;
}

ping_client_handle create_ping_client(int argc, const char * argv[], flag_response * response)
{
	ping_client_handle new = (ping_client_handle)malloc(sizeof(*new));
	new->flags = parse_arguments(argc, argv, response);
	return new;
}

void delete_ping_client(ping_client_handle ping_client)
{
	delete_flags_handle(ping_client->flags);
	free(ping_client);
}

void set_alarm(long x)
{
	alarm(x);
}

void sighandler(int signum) {
	pthread_cancel(current_thread_t);
}

typedef struct thread_data {
	int socket_fd;
	char * start_timestamp;
	bool completed;
} thread_data;

char * timestamp_from_response(char * buffer, size_t timestamp_size)
{
	int index = 0;
	size_t buffer_size = strlen(buffer);
	bool fs_hit = false; //first space hit
	bool ss_hit = false; //second space hit
	char * timestamp = (char *)malloc(sizeof(char) * (timestamp_size + 1));
	for (int i = 0; i < buffer_size; i++)
	{
		char c = buffer[i];
		if (!fs_hit) {
			if (c == ' ') {
				fs_hit = true;
				continue;
			}
		}
		else if (fs_hit && !ss_hit) {
			if (c == ' ') {
				ss_hit = true;
				continue;
			}
		}
		else { //both spaces hit, timestamp
			if (c == '\r') { //end of timestamp
				break;
			}
			else {
				timestamp[index++] = c;
			}
		}
	}
	timestamp[index] = '\0';
	return timestamp;
}

//thread function for receiving datagrams
void * receive_from(void * data)
{
	thread_data * data_handle = (thread_data *)data;
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', BUFFER_SIZE);
	int received = 0;
	struct sockaddr new;
	socklen_t fromlen = sizeof(struct sockaddr);
	while (true) {
		received = recvfrom(data_handle->socket_fd, buffer, BUFFER_SIZE, 0, &new, &fromlen);
		if (received > 0) {
			char * timestamp = timestamp_from_response(buffer, strlen(data_handle->start_timestamp));
			if (strcmp(timestamp, data_handle->start_timestamp) == 0) { //if timestamps match it was a successful ping
				data_handle->completed = true;
				return NULL;
			}
			memset(buffer, '\0', BUFFER_SIZE);
			free(timestamp);
		}
		else if (received < 0) { //message was received
			fprintf(stderr, "receive error: %s\n", strerror(errno));
		}
	}
	return NULL;
}

//return full ping message and also a timestamp 
char * create_ping_message(unsigned int count, char ** timestamp)
{
	char ping[] = "PING";
	char number[20];
	snprintf(number, 20, "%d", count);
	*timestamp = timestamp_from_now();
	//ping, space, seq, space, timestamp, plus 3 for \r\n\0,
	size_t total_size = strlen(ping) + 1 + strlen(number) + 1 + strlen(*timestamp) + 3;
	char * message = (char *)malloc(total_size * sizeof(char));
	message[0] = '\0';
	strcat(message, ping);
	strcat(message, " ");
	strcat(message, number);
	strcat(message, " ");
	strcat(message, *timestamp);
	strcat(message, "\r\n");
	return message;
}

void start_client(ping_client_handle ping_client)
{
	int status;
	struct addrinfo hints, *result;
	int socket_fd;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //make it use IPv4
	hints.ai_socktype = SOCK_DGRAM; //specify UDP
	struct sockaddr_in sa;
	inet_pton(AF_INET, ping_client->flags->server_ip, &(sa.sin_addr));
	hints.ai_addr = (struct sockaddr *)&(sa.sin_addr);
	
	//get address info for server
	if ((status = getaddrinfo(NULL, ping_client->flags->server_port, &hints, &result)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(4);
	}
	
	//create a new socket
	if ((socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == -1) {
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		exit(5);
	}
	
	char ip[INET6_ADDRSTRLEN];
	getIP(result->ai_addr, ip);
	
	//print the IP address for the output
	printf("PING %s\n", ping_client->flags->server_ip);
	
	//initialize stats object with the count
	stats_handle stats = create_stats(ping_client->flags->count);
	
	unsigned int count = 1;
	while (count <= ping_client->flags->count)
	{
		char * start_timestamp;
		char * message = create_ping_message(count, &start_timestamp);
		if ((status = sendto(socket_fd, (void*)message, strlen(message) + 1, 0, result->ai_addr, result->ai_addrlen)) == -1) {
			fprintf(stderr, "connect error: %s\n", strerror(errno));
			free(message);
			exit(6);
		}
		free(message);
		
		//set alarm for timeout on the packet
		signal(SIGALRM, sighandler);
		set_alarm(ping_client->flags->timeout);
		
		//create new thread for receive procedure
		pthread_t worker_thread;
		
		thread_data thread_data;
		thread_data.socket_fd = socket_fd;
		thread_data.start_timestamp = start_timestamp;
		thread_data.completed = false;
		
		pthread_attr_t attr_t;
		pthread_attr_init(&attr_t);
		pthread_attr_setdetachstate(&attr_t, PTHREAD_CREATE_JOINABLE);
		
		if ((status = pthread_create(&worker_thread, &attr_t, receive_from, &thread_data)) != 0) {
			fprintf(stderr, "error joining thread: %s\n", strerror(errno));
			exit(8);
		}
		
		//set global variable so sighandler can cancel the thread
		current_thread_t = worker_thread;
		
		//join thread
		if ((status = pthread_join(worker_thread, NULL)) != 0) {
			fprintf(stderr, "error joining thread: %s\n", strerror(errno));
			exit(8);
		}
		
		//process thread completion/cancellation
		char * end_timestamp = timestamp_from_now();
		long rtt = difference_between_timestamps(start_timestamp, end_timestamp);
		if (thread_data.completed) {
			//assert(rtt >= 0);
			insert_rtt_for_seq(stats, rtt, count);
			printf("PONG %s: seq=%d time=%ld ms\n", ping_client->flags->server_ip, count, rtt);
		}
		else {
			//-1 indicates it was lost
			insert_rtt_for_seq(stats, LOST_RTT, count);
		}
		free(start_timestamp);
		free(end_timestamp);
		
		count++;
		long period_left = (ping_client->flags->period * 1000) - rtt; //in milliseconds
		if (count <= ping_client->flags->count && (period_left > 0)) { //if there are still pings to be sent & time left
			long period_to_sleep = period_left * 1000000; //convert to nanoseconds
			struct timespec t1;
			t1.tv_sec = period_to_sleep / 1000000000;
			t1.tv_nsec = period_to_sleep - (t1.tv_sec * 1000000000);
			if (nanosleep(&t1 , NULL) < 0) {
				fprintf(stderr, "nanosleep failed: %s\n", strerror(errno));
			}
		}
	}
	//finish
	ping_client_done(stats); //put end_timestamp on stats
	set_alarm(0);
	current_thread_t = 0;
	close(socket_fd);
	freeaddrinfo(result);
	
	//print final stats
	get_final_summary(stats, ping_client->flags->server_ip);
	delete_stats(stats);
}
