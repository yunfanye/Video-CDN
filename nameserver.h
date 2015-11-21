#ifndef __NAMESERVER__
#define __NAMESERVER__

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>

int bind_udp(fd_set* read_set, char* ip, int port);


#endif