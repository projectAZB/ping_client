//
//  flags.h
//  project3
//
//  Created by Adam on 2/25/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#ifndef flags_h
#define flags_h

enum flag_response {
	valid_flag,
	invalid_flag,
	duplicate_flag
};

typedef enum flag_response flag_response;

struct flags {
	char * server_ip;
	char * server_port;
	long count;
	long period;
	long timeout;
};

typedef struct flags flags;

typedef flags * flags_handle;

flags_handle parse_arguments(int argc, const char * argv[], flag_response * response);

void delete_flags_handle(flags_handle flags);

void print_flags(flags_handle flags);

#endif
