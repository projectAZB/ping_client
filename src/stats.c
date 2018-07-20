//
//  stats.c
//  project3
//
//  Created by Adam on 3/3/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#include "stats.h"

#include <string.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

struct stats {
	unsigned int count;
	long * rtts; //if !completed, rtts[i] = -1
	unsigned int lost;
	long start_timestamp;
	long end_timestamp;
};

stats_handle create_stats(unsigned int count)
{
	stats_handle new = (stats_handle)malloc(sizeof(*new));
	new->rtts = (long *)malloc(sizeof(long) * count);
	new->count = count;
	new->start_timestamp = long_from_timestamp(timestamp_from_now());
	new->lost = 0;
	return new;
}

void delete_stats(stats_handle stats)
{
	free(stats->rtts);
	free(stats);
}

char * timestamp_from_now()
{
	struct timeb time;
	ftime(&time);
	char seconds[50];
	snprintf(seconds, 50, "%lu", time.time);
	char milli[50];
	snprintf(milli, 50, "%d", time.millitm);
	size_t millilen = strlen(milli);
	size_t tmstp_size = strlen(seconds) + 4;
	char * timestamp = (char *)malloc(sizeof(char) * tmstp_size);
	timestamp[0] = '\0';
	strcat(timestamp, seconds);
	if (millilen == 1) {
		strcat(timestamp, "00");
	}
	else if (millilen == 2) {
		strcat(timestamp, "0");
	}
	strcat(timestamp, milli);
	return timestamp;
}

long long_from_timestamp(char * timestamp)
{
	char *ptr;
	long value = strtol(timestamp, &ptr, 10);
	return value;
}

long difference_between_timestamps(char * t1, char * t2)
{
	return long_from_timestamp(t2) - long_from_timestamp(t1);
}

//rtt will be -1 if not completed
void insert_rtt_for_seq(stats_handle stats, int rtt, unsigned int seq)
{
	stats->rtts[seq - 1] = rtt;
	if (rtt == -1 ) {
		stats->lost++;
	}
}

void ping_client_done(stats_handle stats)
{
	stats->end_timestamp = long_from_timestamp(timestamp_from_now());
}

long min(stats_handle stats)
{
	if (stats->lost == stats->count) return 0; //no values in array
	long min = LONG_MAX;
	for (int i = 0; i < stats->count; i++)
	{
		long ith_rtt = stats->rtts[i];
		min = (ith_rtt >= 0 && ith_rtt < min) ? ith_rtt : min;
	}
	return min;
}

long max(stats_handle stats)
{
	if (stats->lost == stats->count) return 0; //no values in array
	long max = LONG_MIN;
	for (int i = 0; i < stats->count; i++)
	{
		long ith_rtt = stats->rtts[i];
		max = (ith_rtt >= 0 && ith_rtt > max) ? ith_rtt : max;
	}
	return max;
}

int received(stats_handle stats)
{
	return stats->count - stats->lost;
}

long avg(stats_handle stats)
{
	if (received(stats) == 0) return 0;
	long sum = 0;
	//int j = 0;
	for (int i = 0; i < stats->count; i++)
	{
		long ith_rtt = stats->rtts[i];
		if (ith_rtt != -1) {
			sum += ith_rtt;
			//j++;
		}
	}
	//assert(j == received(stats));
	return sum / received(stats);
}

double percent_loss(stats_handle stats)
{
	double percent = (double)stats->lost / stats->count;
	return percent * 100;
}

long total_time(stats_handle stats)
{
	return stats->end_timestamp - stats->start_timestamp;
}

void get_final_summary(stats_handle stats, char * ip_address)
{
	printf("--- %s ping statistics ---\n", ip_address);
	printf("%d transmitted, %d received, %.1f%% loss, time %ld ms\n", stats->count, received(stats), percent_loss(stats), total_time(stats));
	printf("rtt min/avg/max = %ld/%ld/%ld\n", min(stats), avg(stats), max(stats));
}

