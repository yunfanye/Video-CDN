#include "nameserver.h"

char reference_packet[MAX_BUFFER];
int reference_packet_length;

int bind_udp(fd_set* read_set, char* ip, int port){
  int sock;
  struct sockaddr_in temp_addr;
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if(sock==-1){
  	printf("Failed socket\n");
  	return -1;
  }
  
  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  temp_addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &(temp_addr.sin_addr));
  temp_addr.sin_port = htons(port);
  
  if(bind(sock, (struct sockaddr*)&temp_addr, sizeof(struct sockaddr_in))<0){
  	printf("Failed bind\n");
  	return -1;
  }
  FD_SET(sock, read_set);
  return sock;
}

int main(int argc, char* argv[]) {
	int round_robin = 0;
	char* log_filename;
	char* ip;
	int port;
	char* servers_filename;
	char* LSAs;
	if(argc==6){
		log_filename = argv[1];	
		ip = argv[2];
		port = atoi(argv[3]);
		servers_filename = argv[4];
		LSAs = argv[5];
	}else if(argc==7){
		round_robin = 1;
		log_filename = argv[2];	
		ip = argv[3];
		port = atoi(argv[4]);
		servers_filename = argv[5];
		LSAs = argv[6];
	}else{
		printf("usage: ./nameserver [-r] <log> <ip> <port> <servers> <LSAs>\n");
    	exit(0);
	}
	fd_set read_set;
	fd_set ready_read_set;
	int socket = bind_udp(&read_set, ip, port);
	if(socket<0){
		printf("Error bind udp\n");
		exit(-1);
	}
	round_robin_count = 0;
	parse_server_file(servers_filename);
	parse_LSAs_file(LSAs);
	build_routing_table(round_robin);
	// print_routing_table();
	// print_topo();
	open_log_file(log_filename);
	struct packet* temp_packet = make_query_packet("video.cs.cmu.edu");
	int temp_length;
	serialize(temp_packet, reference_packet, &reference_packet_length);
	// printf("Reference packet length: %d\n", reference_packet_length);
	// printf("Printing reference_packet..........................\n");
	// print_packet(temp_packet);
	// print_serialized_packet(reference_packet, reference_packet_length);
	// printf("End printing reference_packet..........................\n");
	int ready_number = -1;
	while(1){
		ready_read_set = read_set;
		ready_number = select(socket+1, &ready_read_set, NULL, NULL, NULL);
		if(ready_number<0){
			printf("Select error\n");
			close(socket);
			exit(-1);
		}
		else if(ready_number==1){
			struct sockaddr_in from;
			socklen_t from_length = sizeof(from);
			char buffer[MAX_BUFFER];
			memset(buffer, 0, MAX_BUFFER);
			int ret = recvfrom(socket, buffer, MAX_BUFFER,0, (struct sockaddr*)&from, &from_length);
			if(ret<0){
				printf("Error recvfrom\n");
				exit(-1);
			}else{
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(from.sin_addr), addr, INET_ADDRSTRLEN);
				printf("client address: %s\n", addr);
				// print_serialized_packet(buffer, ret);
				// int jj;
				// for(jj=0;jj<reference_packet_length-sizeof(uint16_t);jj++){
				// 	if(memcmp(buffer+sizeof(uint16_t)+jj, reference_packet+sizeof(uint16_t)+jj, 1)!=0){
				// 		printf("differ: %d, %c, %c\n", jj, (char*)(buffer+sizeof(uint16_t)+jj), (char*)(reference_packet+sizeof(uint16_t)+jj));
				// 	}
				// }
				if(memcmp(buffer+sizeof(uint16_t), reference_packet+sizeof(uint16_t), reference_packet_length-sizeof(uint16_t))==0){
					// generate response
					printf("correct dns\n");
					char* response_ip = best_server(addr, round_robin);
					if(response_ip!=NULL){
						printf("find best server: %s\n", response_ip);
						int response_length = -1;
						char* response = make_response_packet(buffer, response_ip, &response_length);
						dns_log(addr, "video.cs.cmu.edu", response_ip);
						sendto(socket, response, response_length, 0, (struct sockaddr *)&from, from_length);
						// print_serialized_packet(response, response_length);
						free(response);
					}
					else{
						printf("invalid client in topo\n");
						int response_length;
						char* response = make_error_response_packet(buffer, &response_length);
						sendto(socket, response, response_length, 0, (struct sockaddr *)&from, from_length);
						// print_serialized_packet(response, response_length);
						free(response);
					}
				}
				else{
					printf("wrong dns\n");
					// generate error response
					int response_length = -1;
					char* response = make_error_response_packet(buffer, &response_length);
					sendto(socket, response, response_length, 0, (struct sockaddr *)&from, from_length);
					print_serialized_packet(response, response_length);
					free(response);
				}
			}
		}
		FD_CLR(socket, &read_set);
		FD_SET(socket, &read_set);
	}
	return 0;
}