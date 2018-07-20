//
//  ping_client.h
//  project3
//
//  Created by Adam on 2/26/18.
//  Copyright © 2018 Adam. All rights reserved.
//

#ifndef ping_client_h
#define ping_client_h

#include "flags.h"

struct ping_client;

typedef struct ping_client ping_client;

typedef ping_client * ping_client_handle;

ping_client_handle create_ping_client(int argc, const char * argv[], flag_response * response);
void delete_ping_client(ping_client_handle ping_client);
void start_client(ping_client_handle ping_client);

#endif
