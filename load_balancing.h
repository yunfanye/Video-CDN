#ifndef __LOAD_BALANCING__
#define __LOAD_BALANCING__

#include <string.h>

struct node{
	char name[MAX_BUFFER];
	struct node_pointer* neighbors;
	int seq_num;
	int visited;
	struct node* next;
};

struct node_pointer{
	struct node* p_node;
	struct neighbor* next;
};

struct routing_table_entry{
	struct node* client;
	struct node* server;
	struct routing_table_entry* next;
};

struct node* nodes;
struct node_pointer* servers;
struct node_pointer* clients;
int round_robin_count;
struct routing_table_entry* routing_table;


// find best server and return ip address
char* best_server(char* client_addr, int round_robin);

#endif