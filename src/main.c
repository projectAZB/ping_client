//
//  main.c
//  project3
//
//  Created by Adam on 2/22/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ping_client.h"

int main(int argc, const char * argv[])
{
	if (argc == 6) {
		flag_response response;
		ping_client_handle ping_client = create_ping_client(argc, argv, &response);
		if (response == duplicate_flag) {
			fprintf(stderr, "A duplicate flag was provided, please check your input and try again.\n");
			exit(3);
		}
		else if (response == invalid_flag) {
			fprintf(stderr, "An invalid flag was provided, please check your input and try again.\n");
			exit(4);
		}
		start_client(ping_client);
		delete_ping_client(ping_client);
	}
	else {
		if (argc > 6) {
			fprintf(stderr, "Too many arguments provided, only enter a server port number, server IP address, count of pings to send, a wait interval, and a timeout.\n");
			exit(1);
		}
		else { //argc < 6
			fprintf(stderr, "Not enough arguments provided, you must enter a server port number, server IP address, count of pings to send, a wait interval, and a timeout.\n");
			exit(2);
		}
	}
	
	return 0;
}
