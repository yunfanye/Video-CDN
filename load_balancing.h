#ifndef __LOAD_BALANCING__
#define __LOAD_BALANCING__

#include "mydns.h"

struct node{
	char name[MAX_BUFFER];
	struct node_pointer* neighbors;
	int seq_num;
	int visited;
	struct node* next;
};

struct node_pointer{
	struct node* p_node;
	struct node_pointer* next;
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

int parse_server_file(char* filename);
int parse_LSAs_file(char *filename);
// find best server and return ip address
char* best_server(char* client, int round_robin);
struct node* add_node(char* name, struct node** list);
void add_node_list(struct node_pointer** list, struct node* p_node);
int exist_in_list(struct node_pointer* list, struct node* p_node);
void nearest_server(struct node* client);
struct node* pop_front(struct node_pointer** queue);
void push_back(struct node_pointer** queue, struct node* node);
void print_routing_table();
void print_topo();
void build_routing_table(int round_robin);
void print_servers_clients();

#endif