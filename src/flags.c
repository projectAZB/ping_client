//
//  flags.c
//  project3
//
//  Created by Adam on 2/25/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#include "flags.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void get_flag_value(const char * raw, char * flag, char * value)
{
	size_t raw_length = strlen(raw);
	unsigned int index = 0;
	if (raw[index] == '-' && raw[index + 1] == '-') {
		index = 2; //set past the --
	}
	else {
		fprintf(stderr, "%s has an invalid format\n", raw);
		exit(3);
	}
	unsigned int flag_index = 0;
	for (; index < raw_length; index++)
	{
		if (raw[index] == '=') {
			break;
		}
		flag[flag_index++] = raw[index];
	}
	flag[flag_index] = '\0';
	
	if (index == raw_length) {
		fprintf(stderr, "%s has an invalid format\n", raw);
		exit(3);
	}
	
	index++; //increment past the '=' sign
	unsigned int value_index = 0;
	for (; index < raw_length; index++)
	{
		value[value_index++] = raw[index];
	}
	value[value_index] = '\0';
}

flags_handle create_flags_handle(void)
{
	flags_handle new = (flags_handle)malloc(sizeof(*new));
	new->server_ip = NULL;
	new->server_port = NULL;
	new->count = -1;
	new->period = -1;
	new->timeout = -1;
	return new;
}

#define server_ip_s "server_ip"
#define server_port_s "server_port"
#define count_s "count"
#define period_s "period"
#define timeout_s "timeout"

flag_response add_flag_with_identifier(flags_handle flags, char * identifier, char * value)
{
	if (strcmp(server_ip_s, identifier) == 0) {
		if (flags->server_ip == NULL) {
			char * server_ip = (char *)malloc(sizeof(char) * (strlen(value) + 1));
			strcpy(server_ip, value);
			flags->server_ip = server_ip;
			return valid_flag;
		}
		else {
			return duplicate_flag;
		}
	}
	else if (strcmp(server_port_s, identifier) == 0) {
		if (flags->server_port == NULL) {
			char * server_port = (char *)malloc(sizeof(char) * (strlen(value) + 1));
			strcpy(server_port, value);
			flags->server_port = server_port;
			return valid_flag;
		}
		else {
			return duplicate_flag;
		}
	}
	else if (strcmp(count_s, identifier) == 0) {
		if (flags->count == -1) {
			long count;
			char * ptr; //this should be empty
			count = strtol(value, &ptr, 10);
			if (strlen(ptr) != 0) { //there was a wrong format
				fprintf(stderr, "%s is in an invalid format for %s\n", value, identifier);
				return invalid_flag;
			}
			flags->count = count;
			return valid_flag;
		}
		else {
			return duplicate_flag;
		}
	}
	else if (strcmp(period_s, identifier) == 0) {
		if (flags->period == -1) {
			long period;
			char * ptr; //this should be empty
			period = strtol(value, &ptr, 10);
			if (strlen(ptr) != 0) { //there was a wrong format
				fprintf(stderr, "%s is in an invalid format for %s\n", value, identifier);
				return invalid_flag;
			}
			flags->period = period / 1000;
			return valid_flag;
		}
		else {
			fprintf(stderr, "%s is in an invalid format for %s\n", value, identifier);
			return duplicate_flag;
		}
	}
	else if (strcmp(timeout_s, identifier) == 0) {
		if (flags->timeout == -1) {
			long timeout;
			char * ptr; //this should be empty
			timeout = strtol(value, &ptr, 10);
			if (strlen(ptr) != 0) { //there was a wrong format
				fprintf(stderr, "%s is in an invalid format for %s\n", value, identifier);
				return invalid_flag;
			}
			flags->timeout = timeout / 1000;
			return valid_flag;
		}
		else {
			return duplicate_flag;
		}
	}
	else {
		return invalid_flag;
	}
}

flags_handle parse_arguments(int argc, const char * argv[], flag_response * response)
{
	flags_handle flags = create_flags_handle();
	for (int i = 1; i < argc; i++)
	{
		char flag[50];
		char value[50];
		get_flag_value(argv[i], flag, value);
		flag_response resp = add_flag_with_identifier(flags, flag, value);
		switch ((*response = resp)) {
			case duplicate_flag:
				return flags;
			case invalid_flag:
				return flags;
			default:
				break;
		}
	}
	return flags;
}

void delete_flags_handle(flags_handle flags)
{
	free(flags->server_ip);
	free(flags->server_port);
	free(flags);
}

void print_flags(flags_handle flags)
{
	printf("Server IP: %s\n", flags->server_ip);
	printf("Server Port: %s\n", flags->server_port);
	printf("Count: %ld\n", flags->count);
	printf("Timeout: %ld\n", flags->timeout);
	printf("Period: %ld\n", flags->period);
}
