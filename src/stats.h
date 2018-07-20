//
//  stats.h
//  project3
//
//  Created by Adam on 3/3/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#ifndef stats_h
#define stats_h

#define LOST_RTT (-1)

struct stats;

typedef struct stats stats;

typedef stats * stats_handle;

stats_handle create_stats(unsigned int count);
void delete_stats(stats_handle stats);

void insert_rtt_for_seq(stats_handle stats, int rtt, unsigned int seq);
char * timestamp_from_now(void);
long long_from_timestamp(char * timestamp);
long difference_between_timestamps(char * t1, char * t2);
void ping_client_done(stats_handle stats); //adds end_timestamp for stats

void get_final_summary(stats_handle stats, char * ip_address);

#endif
