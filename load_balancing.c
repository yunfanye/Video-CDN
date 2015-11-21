#include "load_balancing.h"

// return -1 if error encountered
int parse_server_file(char* filename){
	FILE *f;
	f = fopen(filename, "r");
	if(!f){
		return -1;
	}
	char buffer[MAX_BUFFER];
	while(fgets(buffer, MAX_BUFFER, f)){
		buffer[strlen(buffer)-1] = '\0';
		// TODO: add server
	}
	fclose(f);
	return 1;
}

int parse_LSAs_file(char *filename) {
	FILE *f;
	f = fopen(filename, "r");
	if(!f){
		return -1;
	}
	char buffer[MAX_BUFFER];
	char sender[MAX_BUFFER];
	int seq_num;
	char neighbors[MAX_BUFFER];
	while(fgets(buffer, MAX_BUFFER, f)){
		sscanf(buffer, "%s %d %s", sender, &seq_num, neighbors);
		struct node* node_p = node_exist();
		if(node_p==NULL){
			// add node
		}
		if(node_p->seq_num>=seq_num){
			continue;
		}
		node_p->seq_num = seq_num;
		free(node_p->neighbors);
		node_p->neighbors = NULL;

		neighbor = strtok(neighbors, ",");
		while(neighbor) {
			// add node neighbor
			neighbor = strtok(NULL, ",");
		}
	}
	fclose(f);
	return 1;
}

// find best server and return ip address
char* best_server(char* client, int round_robin){
	char* return_str = NULL;
	if(round_robin==1){
		uint32_t size = mlist_length(nodes);
		node_t* node = (node_t*)mlist_getdata(nodes, query_count % size);
		query_count++;
		return node->name;
	}
	else{
		MList* list = mlist_find_custom(routing_table, clit_name, find_entry_by_name);
		rt_t* entry = (rt_t*)list->data;
		return entry->serv->name;
	}
	return return_str;
}