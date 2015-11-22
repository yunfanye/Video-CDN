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
		struct node* p_node = add_node(buffer, nodes);
		add_node_list(&servers, p_node);
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
		struct node* node_p = add_node(sender, nodes);
		if(exist_in_list(servers, node_p)<0){
			if(strncmp(node_p->name, "router", strlen("router"))==0 || strlen(node_p->name)<strlen("router")){
				add_node_list(clients, node_p);
			}
		}
		if(node_p->seq_num<seq_num){
			node_p->seq_num = seq_num;
			free(node_p->neighbors);
			node_p->neighbors = NULL;
			neighbor = strtok(neighbors, ",");
			while(neighbor) {
				struct node* p_neighbor = add_node(neighbor, nodes);
				add_node_list(&(node_p->neighbors), p_neighbor);
				neighbor = strtok(NULL, ",");
			}
		}
	}
	fclose(f);
	return 1;
}

// find best server and return ip address
char* best_server(char* client, int round_robin){
	char* return_str = NULL;
	if(round_robin==1){
		// find node by round_robin
		// count nodes length
		int total_count = 0;
		struct node* temp = nodes;
		while(temp){
			total_count++;
			temp = temp->next;
		}
		int index = round_robin_count%total_count;
		struct node* temp = nodes;
		int count = 0;
		while(temp){
			if(count==index){
				break;
			}
			count++;
			temp = temp->next;
		}
		round_robin_count++;
		return_str = temp->name;
	}
	else{
		// TODO
		return_str = node->name;
	}
	return return_str;
}

struct node* add_node(char* name, struct node* list) {
	struct node* temp = list;
	while(temp){
		if(strcmp(temp->name, name)==0){
			break;
		}
	    temp = temp->next;
	}
	if(temp==NULL){
		temp = (struct node*)malloc(sizeof(struct node));
		strcpy(temp->name, name);
		temp->seq_num = -1;
		temp->neighbors = NULL;
		temp->next = list;
		temp->visited = -1;
		list = temp;
	}
	return temp;
}


// add node to list if node is not in the list
// if node already in list, do nothing
void add_node_list(struct node_pointer** list, struct node* p_node){
	struct node_pointer* temp = *list;
	while(temp){
		if(temp->p_node==p_node){
			break;
		}
	    temp = temp->next;
	}
	if(temp==NULL){
		temp = (struct node_pointer*)malloc(sizeof(struct node_pointer));
		temp->p_node = p_node;
		temp->next = *list;
		*list = temp;
	}
}

int exist_in_list(struct node_pointer* list, struct node* p_node){
	struct node_pointer* temp = list;
	while(temp){
		if(temp->p_node==p_node){
			return 1;
		}
	    temp = temp->next;
	}
	return -1;
}

// Use BFS search to find the nearest server for a client
void nearest_server(struct node* client){
	// mark all nodes as unvisited.
	struct node* temp = nodes;
	while(temp){
		temp->visited = 0;
		temp = temp->next;
	}
	client->visited = 1;

	struct node_pointer* queue = NULL;
	push_back(queue, client);
	int flag = 1;
	while(flag==1){
		struct node* current_node = pop_front(queue);
		struct node_pointer* temp = current_node->neighbors;
		while(temp){
			if(temp->p_node->visited==0){
				if(exist_in_list(servers, temp->p_node)>0){
					flag = 0;
					break;
				}
				else{
					push_back(queue, temp->p_node);
				}
			}
			temp->p_node->visited = 1;
			temp = temp->next;
		}
	}
	if(temp==NULL){
		printf("Faital Error, not found server\n");
		exit(-1);
	}
	struct routing_table_entry* rte = (struct routing_table_entry*)malloc(sizeof(struct routing_table_entry));
	rte->next = routing_table;
	routing_table = rte;
	rte->client = client;
	rte->server = temp->p_node;
}

struct node* pop_front(struct node_pointer** queue){
	struct node_pointer* queue_head = *queue;
	struct node_pointer* return_p = NULL;
	if(queue_head==NULL){
		return NULL;
	}
	if(queue_head->next==NULL){
		return_p = queue_head->p_node;
		free(queue_head);
		*queue = NULL;
	}
	else{
		return_p = queue_head->p_node;
		*queue = queue_head->next;
		free(queue_head);
	}
	return return_p;
}
void node_pointer* push_back(struct node_pointer** queue, struct node* node){
	struct node_pointer* queue_head = *queue;
	if(queue_head==NULL){
		*queue = (struct node_pointer*)malloc(sizeof(struct node_pointer));
		(*queue)->p_node = node;
		(*queue)->next = NULL;
		return;
	}
	if(queue_head->next==NULL){
		queue_head->next = (struct node_pointer*)malloc(sizeof(struct node_pointer));
		queue_head->next->p_node = node;
		queue_head->next->next = NULL;
		return;
	}
	struct node_pointer* temp = *queue;
	while(temp->next){
		temp = temp->next;
	}
	temp->next = (struct node_pointer*)malloc(sizeof(struct node_pointer));
	temp->next->p_node = node;
	temp->next->next = NULL;
	return;
}
