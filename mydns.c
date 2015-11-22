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

void make_addrinfo(struct addrinfo** res){
	*res = (struct addrinfo*)malloc(sizeof(struct addrinfo));
	memset(*res, 0, sizeof(struct addrinfo));
	(*res)->ai_flags = AI_PASSIVE;
	(*res)->ai_family = AF_INET;
	(*res)->ai_socktype = SOCK_STREAM;
	(*res)->ai_protocol = 0;
	(*res)->ai_addrlen = sizeof(struct sockaddr_in);
	(*res)->ai_addr = malloc(sizeof(struct sockaddr_in));
	(*res)->ai_canonname = NULL;
	(*res)->ai_next = NULL;
}

struct packet* make_query_packet(const char* node){
	struct packet* packet = (struct packet*)malloc(sizeof(struct packet));
	packet->header = (struct header*)malloc(sizeof(struct header));
	packet->question = (struct question*)malloc(sizeof(struct question));

	srand(time(NULL));
	packet->header->ID = htons((uint16_t)rand());
	packet->header->QR = 0;
	packet->header->Opcode = 0;
	packet->header->AA = 0;  
	packet->header->TC = 0;  
	packet->header->RD = 0;
    packet->header->RA = 0;
	packet->header->Z  = 0;
    packet->header->RCODE = 0;   

    packet->header->QDCOUNT = htons(1);
    packet->header->ANCOUNT = htons(0);
    packet->header->NSCOUNT = 0;
    packet->header->ARCOUNT = 0;

	packet->question->QNAME = (char*)malloc(strlen(node)+1);
	memcpy(packet->question->QNAME, node, strlen(node)+1);
	packet->question->QTYPE = htons(1);
	packet->question->QCLASS = htons(1);

	packet->resource_record = NULL;

	return packet;
}
void free_packet(struct packet* packet){
	free(packet->header);

	free(packet->question->QNAME);
	free(packet->question);

	if (packet->resource_record) {
		free(packet->resource_record->NAME);
		free(packet->resource_record);
	}
	free(packet);
}

char* make_response_packet(char* input_buffer, char* dest_addr, int* response_length){
	char* response = (char*)malloc(MAX_BUFFER);
	int length = sizeof(struct header);
	length += strlen((char*)(input_buffer+length))+1 + sizeof(uint16_t)*2;
	memcpy(response, input_buffer, length);
	((struct header*)response)->QR = 1;
	((struct header*)response)->AA = 1;
	((struct header*)response)->ANCOUNT = htons(1);
	char* QNAME = (char*)(response+sizeof(struct header));
	memcpy(response+length, QNAME, strlen(QNAME)+1);
	length += strlen(QNAME)+1;
	(struct resource_record*)(response+length)->TYPE = htons(1);
	(struct resource_record*)(response+length)->CLASS = htons(1);
	(struct resource_record*)(response+length)->TTL = 0;
	(struct resource_record*)(response+length)->RDLENGTH = htons(4);
	length += sizeof(struct resource_record);
	struct sockaddr_in temp_addr;
	inet_pton(AF_INET, dest_addr, &(temp_addr.sin_addr));
	*((uint32_t*)(response+len)) = temp_addr.sin_addr.s_addr;
	length += 4;
	*response_length = length;
	return response;
}

char* make_error_response_packet(char* input_buffer, int* response_length){
	char* response = (char*)malloc(MAX_BUFFER);
	int length = sizeof(struct header);
	length += strlen((char*)(input_buffer+length))+1 + sizeof(uint16_t)*2;
	memcpy(response, input_buffer, length);
	((struct header*)response)->RCODE = htonl(3);
	*response_length = length;
	return response;
}

void serialize(struct packet* packet, char* data, int* length){
	int offset = 0;
	memcpy(data+offset,packet->header, sizeof(struct header));	
	offset += sizeof(struct header);

	memcpy(data+offset, packet->question->QNAME, strlen(packet->question->QNAME)+1);
	offset += strlen(packet->question->QNAME)+1;

	memcpy(data+offset, ((char*)packet->question)+sizeof(char*), sizeof(uint16_t));
	offset += sizeof(uint16_t);

	if (packet->resource_record) {
		memcpy(data+offset, packet->resource_record->NAME, strlen(packet->resource_record->NAME)+1);
		offset += strlen(packet->resource_record->NAME)+1;
		memcpy(data+offset, ((char*)packet->resource_record)+sizeof(char*), sizeof(uint16_t)*6);
		offset += sizeof(uint16_t)*6;
		// For example, the if the TYPE is A and the CLASS is IN, the RDATA field is a 4 octet ARPA Internet address.
		// memcpy(data+offset, packet->resource_record->RDATA, sizeof(uint16_t)*2);
		// offset += sizeof(uint16_t)*2;
	}
	*length = offset;
}

// TODO: need to debug here to find the right place of IP
void parse_response(char* response, struct addrinfo **res, int packet_length){
	char* QNAME = (char*)(response + sizeof(struct header));
	int offest = packet_length + strlen(QNAME)+1 + sizeof(uint16_t)*2; // padding
	char* ip = response + offest;
	((struct sockaddr_in*)(*res)->ai_addr)->sin_addr.s_addr = *(uint32_t*)ip;
	return;
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

	parse_response(buffer, res, length);
	((struct sockaddr_in*)(*res)->ai_addr)->sin_port = htons(atoi(service));
	free_packet(packet);
	return 0;
}
