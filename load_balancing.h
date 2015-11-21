#ifndef __LOAD_BALANCING__
#define __LOAD_BALANCING__

#include <string.h>

struct node {
	char name[MAX_BUFFER];
	struct node* neighbors;
	int seq_num;
	int visited;
};

// find best server and return ip address
char* best_server(char* client_addr, int round_robin);

#endif