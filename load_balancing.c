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
		struct node* p_node = add_node(buffer, &nodes);
		add_node_list(&servers, p_node);
	}
	fclose(f);
	return 1;
}

int parse_LSAs_file(char *filename){
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
		// printf("%s, %d, %s\n", sender, seq_num, neighbors);
		struct node* node_p = add_node(sender, &nodes);
		if(exist_in_list(servers, node_p)<0){
			if(strncmp(node_p->name, "router", strlen("router"))!=0 || strlen(node_p->name)<strlen("router")){
				add_node_list(&clients, node_p);
			}
		}
		if(node_p->seq_num<seq_num){
			node_p->seq_num = seq_num;
			free(node_p->neighbors);
			node_p->neighbors = NULL;
			char* neighbor = strtok(neighbors, ",");
			while(neighbor){
				struct node* p_neighbor = add_node(neighbor, &nodes);
				add_node_list(&(node_p->neighbors), p_neighbor);
				neighbor = strtok(NULL, ",");
			}
		}
	}
	fclose(f);
	return 1;
}

void build_routing_table(int round_robin){
	struct node_pointer* client = clients;
	while(client){
		nearest_server(client->p_node);
		client = client->next;
	}
}

// find best server and return ip address
char* best_server(char* client, int round_robin){
	char* return_str = NULL;
	if(round_robin==1){
		// find node by round_robin
		// count nodes length
		int total_count = 0;
		struct node_pointer* temp = servers;
		while(temp){
			total_count++;
			temp = temp->next;
		}
		int index = round_robin_count%total_count;
		temp = servers;
		int count = 0;
		while(temp){
			if(count==index){
				break;
			}
			count++;
			temp = temp->next;
		}
		round_robin_count++;
		return_str = temp->p_node->name;
	}
	else{
		struct routing_table_entry* rte = routing_table;
		return_str = NULL;
		while(rte){
			if(strcmp(rte->client->name, client)==0){
				return_str = rte->server->name;
				break;
			}
			rte = rte->next;
		}
	}
	return return_str;
}

struct node* add_node(char* name, struct node** list){
	struct node* temp = *list;
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
		temp->next = *list;
		temp->visited = -1;
		*list = temp;
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
	push_back(&queue, client);
	int flag = 1;
	struct node_pointer* temp_np = NULL;
	while(flag==1){
		struct node* current_node = pop_front(&queue);
		temp_np = current_node->neighbors;
		while(temp_np){
			if(temp_np->p_node->visited==0){
				if(exist_in_list(servers, temp_np->p_node)>0){
					flag = 0;
					break;
				}
				else{
					push_back(&queue, temp_np->p_node);
				}
			}
			temp_np->p_node->visited = 1;
			temp_np = temp_np->next;
		}
	}
	if(temp_np==NULL){
		printf("Faital Error, not found server\n");
		exit(-1);
	}
	struct routing_table_entry* rte = (struct routing_table_entry*)malloc(sizeof(struct routing_table_entry));
	rte->next = routing_table;
	routing_table = rte;
	rte->client = client;
	rte->server = temp_np->p_node;
}

struct node* pop_front(struct node_pointer** queue){
	struct node_pointer* queue_head = *queue;
	struct node* return_p = NULL;
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

void push_back(struct node_pointer** queue, struct node* node){
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

void print_routing_table(){
	struct routing_table_entry* temp = routing_table;
	while(temp){
		printf("rte: client: %s, server: %s\n", temp->client->name, temp->server->name);
		temp = temp->next;
	}
}

void print_servers_clients(){
	printf("Clients:\n");
	struct node_pointer* client = clients;
	while(client){
		printf("client: %s\n", client->p_node->name);
		client = client->next;
	}
	printf("Servers:\n");
	struct node_pointer* server = servers;
	while(server){
		printf("server: %s\n", server->p_node->name);
		server = server->next;
	}
}

void print_topo(){
	struct node* current_node = nodes;
	while(current_node){
		printf("Current node: %s\n", current_node->name);
		struct node_pointer* np = current_node->neighbors;
		while(np){
			printf("neighbor: %s\n", np->p_node->name);
			np = np->next;
		}
		printf("\n");
		current_node = current_node->next;
	}
}

// int main(int argc, char** argv){
// 	if(argc<2){
// 		printf("need more args!\n");
// 		return -1;
// 	}
// 	// printf("strlen: %lu\n", strlen("router"));
// 	round_robin_count = 0;
// 	parse_server_file(argv[1]);
// 	parse_LSAs_file(argv[2]);
// 	print_topo();
// 	print_servers_clients();
// 	int round_robin = 1;
// 	build_routing_table(round_robin);
// 	print_routing_table();
// 	printf("route: 1.0.0.1  --->  %s\n", best_server("1.0.0.1", round_robin));
// 	printf("route: 2.0.0.1  --->  %s\n", best_server("2.0.0.1", round_robin));
// 	printf("route: 3.0.0.1  --->  %s\n", best_server("3.0.0.1", round_robin));
// 	printf("route: 1.0.0.1  --->  %s\n", best_server("1.0.0.1", round_robin));
// 	printf("route: 2.0.0.1  --->  %s\n", best_server("2.0.0.1", round_robin));
// 	printf("route: 3.0.0.1  --->  %s\n", best_server("3.0.0.1", round_robin));
// 	return 1;
// }