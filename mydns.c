#include "mydns.h"


int init_mydns(const char *dns_ip, unsigned int dns_port){
	dns_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(dns_socket<0){
		printf("Init DNS failed!\n");
		return -1;
	}

	// solve the address already in use problem
	int yes=1;
    if (setsockopt(dns_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
        printf("setsockopt failed!\n");
        return -1;
    }
	int ret = bind(dns_socket, (struct sockaddr*)(&dns_server_addr), sizeof(dns_server_addr));
	if(ret<0){
		printf("DNS bind failed!\n");
		return -1;
	}

	dns_server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, dns_ip, &(dns_server_addr.sin_addr));
	dns_server_addr.sin_port = htons(dns_port);
	return 0;
}

int resolve(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res){
	make_addrinfo(res);
	struct packet* packet = make_query_packet(node);

	char data[MAX_BUFFER];
	int length;
	serialize(packet, data, &length);
	// send packet to DNS server 
	sendto(dns_socket, data, length, 0, (struct sockaddr *)&dns_server_addr, sizeof(dns_server_addr));
	char buffer[MAX_BUFFER];
	struct sockaddr_in from;
	socklen_t from_length = sizeof(from);
	// receive from DNS server
	int ret = recvfrom(dns_socket, buffer, MAX_BUFFER, 0, (struct sockaddr *)&from, &from_length);
	if(ret<0){
		printf("Failed receive from DNS server\n");
		free_packet(packet);
		return -1;
	}
	int RCODE = parse_response(buffer, res, length);
	((struct sockaddr_in*)(*res)->ai_addr)->sin_port = htons(atoi(service));
	((struct sockaddr_in*)(*res)->ai_addr)->sin_family = AF_INET;
	free_packet(packet);
	if(RCODE!=0){
		return -1;
	}
	return 0;
}

// // Test for some of the functions in mydns.c
// int main(){
// 	// printf("%d, %d\n", ntohl(3), ntohs(3));
// 	struct packet* query_packet = make_query_packet("video.cs.cmu.edu");
// 	print_packet(query_packet);

// 	char data[MAX_BUFFER];
// 	int length;
// 	serialize(query_packet, data, &length);

// 	free_packet(query_packet);
// 	init_mydns("127.0.0.1", 12345);
// 	struct addrinfo* result;
// 	int rc = resolve("video.cs.cmu.edu", "8080", NULL, &result);
// 	struct sockaddr_in *ipv4 = (struct sockaddr_in*)result->ai_addr;
// 	char ipAddress[INET_ADDRSTRLEN];
// 	inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);
// 	printf("return code: %d, The IP address is: %s\n", rc, ipAddress);
// 	freeaddrinfo(result);
// 	rc = resolve("baidu.com", "8080", NULL, &result);
// 	ipv4 = (struct sockaddr_in*)result->ai_addr;
// 	inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);
// 	printf("return code: %d, The IP address is: %s\n", rc, ipAddress);
// 	freeaddrinfo(result);
// 	// struct packet* response_packet = make_response_packet();
// 	// struct packet* err_packet = make_error_response_packet();
// 	return 1;
// }
