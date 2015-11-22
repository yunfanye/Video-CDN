#include "nameserver.h"
#include "mydns.h"
#include "load_balancing.h"

static char ref_pkt[BUFSIZE];
static int ref_pkt_len;
static char* serve_hostname = "video.cs.cmu.edu";

int bind_udp(fd_set* read_set, char* ip, int port){
  int socket;
  struct sockaddr_in temp_addr;
  socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if(socket==-1){
  	printf("Failed socket\n");
  	return -1;
  }
  
  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  temp_addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &(temp_addr.sin_addr));
  temp_addr.sin_port = htons(port);
  
  if(bind(socket, (struct sockaddr*)&temp_addr, sizeof(struct sockaddr_in))<0){
  	printf("Failed bind\n");
  	return -1;
  }
  FD_SET(socket, read_set);
  return socket;
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
	parse_servers(servers);
	if(round_robin==1){

	}else{
		parse_LSA(LSAs);
	}
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
			int ret = recvfrom(socket, buffer, MAX_BUFFER,0, (struct sockaddr*)&from, &from_length)
			if(ret<0){
				printf("Error recvfrom\n");
				exit(-1);
			}else{
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(from.sin_addr), addr, INET_ADDRSTRLEN);
				printf("client address: %s\n", addr);
				if(memcmp(buffer+sizeof(uint16_t), ref_pkt+sizeof(uint16_t), ref_pkt_len-sizeof(uint16_t))==0){
					// generate response
					char* response_ip = best_server(addr, round_robin);
					int response_length = -1;
					char* response = make_response_packet(buffer, response_ip, &response_length);
					sendto(socket, response, response_length, 0, (struct sockaddr *)&from, from_length);
					free(response);
				}
				else{
					// generate error response
					char* response = make_error_response_packet(buffer, &response_length);
					sendto(socket, response, response_length, 0, (struct sockaddr *)&from, from_length);
					free(response);
				}
			}
		}
		FD_CLR(socket, &read_set);
		FD_SET(socket, &read_set);
	}
	return 0;
}