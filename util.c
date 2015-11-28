#include "util.h"

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

void print_packet(struct packet* packet){
	if(packet==NULL){
		printf("NULL packet\n");
		return;
	}
	if(packet->header==NULL){
		printf("packet has no header\n");
	}
	else{
		printf("Header:\nID: %d, QR: %d, Opcode: %d, AA: %d, TC: %d, RD: %d, RA: %d, Z:%d, RCODE: %d, QDCOUNT: %d, ANCOUNT: %d, NSCOUNT: %d, ARCOUNT: %d\n",
			ntohs(packet->header->ID), packet->header->QR, packet->header->Opcode, packet->header->AA,
			packet->header->TC, packet->header->RD, packet->header->RA,
			packet->header->Z, ntohl(packet->header->RCODE), ntohs(packet->header->QDCOUNT), 
			ntohs(packet->header->ANCOUNT), packet->header->NSCOUNT, packet->header->ARCOUNT);
	}
	if(packet->question==NULL){
		printf("packet has no question part\n");
	}
	else{
		printf("Question:\nQNAME: %s, QTYPE: %d, QCLASS: %d\n",
			packet->question->QNAME, ntohs(packet->question->QTYPE), ntohs(packet->question->QCLASS));
	}
	if(packet->resource_record==NULL){
		printf("packet has no resource record part\n");
	}
	else{
		printf("Resource record:\nNAME: %s, TYPE: %d, CLASS: %d, TTL: %d, RDLENGTH: %d, RDATA: %d\n",
			packet->resource_record->NAME, ntohs(packet->resource_record->TYPE), ntohs(packet->resource_record->CLASS), 
			ntohl(packet->resource_record->TTL), ntohs(packet->resource_record->RDLENGTH), ntohs(packet->resource_record->RDATA));
	}
}

struct packet* make_query_packet(const char* node){
	struct packet* packet = (struct packet*)malloc(sizeof(struct packet));
	packet->header = (struct header*)malloc(sizeof(struct header));
	packet->question = (struct question*)malloc(sizeof(struct question));

	srand(time(NULL));
	packet->header->ID = htons((uint16_t)rand());
	packet->header->QR = 0;
	packet->header->Opcode = 0;
	// packet->header->AA = 0;
	packet->header->AA = 1;
	// TODO: Here might be a problem
	packet->header->TC = 0;  
	packet->header->RD = 0;
    packet->header->RA = 0;
	packet->header->Z  = 0;
    packet->header->RCODE = htonl(0);   

    packet->header->QDCOUNT = htons(1);
    packet->header->ANCOUNT = 0;
    packet->header->NSCOUNT = 0;
    packet->header->ARCOUNT = 0;

	packet->question->QNAME = (char*)malloc(strlen(node)+2);
	char temp_name_buffer[strlen(node)+1];
	memcpy(temp_name_buffer, node, strlen(node));
	temp_name_buffer[strlen(node)] = '.';
	char* p = packet->question->QNAME;
	int dot = 0;
	int i;
	for(i=0;i<=strlen(node);i++){
		if(temp_name_buffer[i]=='.'){
			*p = (i-dot);
			p++;
			for(;dot<i;dot++){
				*p = temp_name_buffer[dot];
				p++;
			}
			dot++;
		}
	}
	*p = '\0';
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
	char* QNAME = (char*)(input_buffer+sizeof(struct header));

	length += strlen(QNAME)+1 + sizeof(uint16_t)*2;
	memcpy(response, input_buffer, length);
	((struct header*)response)->QR = 1;
	((struct header*)response)->AA = 1;
	((struct header*)response)->ANCOUNT = htons(1);
	
	memcpy(response+length, QNAME, strlen(QNAME)+1);

	length += strlen(QNAME)+1;
	*(uint16_t*)(response+length) = htons(1);
	*(uint16_t*)(response+length+sizeof(uint16_t)) = htons(1);
	*(uint32_t*)(response+length+sizeof(uint32_t)*2) = htonl(0);
	*(uint16_t*)(response+length+sizeof(uint16_t)*4) = htons(4);
	struct sockaddr_in temp_addr;
	inet_pton(AF_INET, dest_addr, &(temp_addr.sin_addr));
	*(uint32_t*)(response+length+sizeof(uint16_t)*5) = (temp_addr.sin_addr.s_addr);
	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(temp_addr.sin_addr), ipAddress, INET_ADDRSTRLEN);
	// printf("make packet, the IP address is: %s, %s, %d, %d\n", dest_addr, ipAddress, temp_addr.sin_addr.s_addr, htonl(temp_addr.sin_addr.s_addr));
	// printf("%d, %d, %d, %d, %d, %d, %d, %d\n", 
	// 	((struct header*)response)->QR,
	// 	((struct header*)response)->AA,
	// 	((struct header*)response)->ANCOUNT,
	// 	*(uint16_t*)(response+length),
	// 	*(uint16_t*)(response+length+2),
	// 	*(uint16_t*)(response+length+4),
	// 	*(uint16_t*)(response+length+6),
	// 	*(uint32_t*)(response+length+8)
	// 	);
	length += sizeof(uint16_t)*7;
	*response_length = length;
	return response;
}

char* make_error_response_packet(char* input_buffer, int* response_length){
	char* response = (char*)malloc(MAX_BUFFER);
	int length = sizeof(struct header);
	length += strlen((char*)(input_buffer+length))+1 + sizeof(uint16_t)*2;
	memcpy(response, input_buffer, MAX_BUFFER);
	// struct resource_record* temp = ;
	// *(struct resource_record**)(response+length) = NULL;
	// printf("here length: %d, %p\n", length, response+length);
	// ((struct header*)response)->RCODE = htons(3);
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

	memcpy(data+offset, ((char*)(packet->question))+sizeof(char*), sizeof(uint16_t)*2);
	offset += sizeof(uint16_t)*2;

	if(packet->resource_record){
		memcpy(data+offset, packet->resource_record->NAME, strlen(packet->resource_record->NAME)+1);
		offset += strlen(packet->resource_record->NAME)+1;
		memcpy(data+offset, ((char*)packet->resource_record)+sizeof(char*), sizeof(uint16_t)*7);
		offset += sizeof(uint16_t)*7;
		// For example, the if the TYPE is A and the CLASS is IN, the RDATA field is a 4 octet ARPA Internet address.
		// memcpy(data+offset, packet->resource_record->RDATA, sizeof(uint16_t)*2);
		// offset += sizeof(uint16_t)*2;
	}
	*length = offset;
}

void print_serialized_packet(char* packet, int packet_length){
	int length = 0;
	printf("ID: %d\n", ntohs(((struct header*)packet)->ID));
	printf("QR: %d\n", (((struct header*)packet)->QR));
	printf("Opcode: %d\n", (((struct header*)packet)->Opcode));
	printf("AA: %d\n", (((struct header*)packet)->AA));
	printf("TC: %d\n", (((struct header*)packet)->TC));
	printf("RD: %d\n", (((struct header*)packet)->RD));
	printf("RA: %d\n", (((struct header*)packet)->RA));
	printf("Z: %d\n", (((struct header*)packet)->Z));
	printf("RCODE: %d\n", ntohl(((struct header*)packet)->RCODE));
	printf("QDCOUNT: %d\n", ntohs(((struct header*)packet)->QDCOUNT));
	printf("ANCOUNT: %d\n", ntohs(((struct header*)packet)->ANCOUNT));
	printf("NSCOUNT: %d\n", ntohs(((struct header*)packet)->NSCOUNT));
	printf("ARCOUNT: %d\n", ntohs(((struct header*)packet)->ARCOUNT));
	length += sizeof(struct header);
	printf("Question NAME: %s\n", packet+length);
	length += strlen(packet+length)+1 + sizeof(uint16_t)*2;
	// printf("length here: %d, %p, %d\n", length, packet+length, packet_length);
	if(ntohl(((struct header*)packet)->RCODE)!=3&&length<packet_length){
		printf("Resource record NAME: %s\n", packet+length);
		printf("Resource record TYPE: %d\n", ntohs(*(uint16_t*)(packet+length+strlen(packet+length)+1+sizeof(uint16_t)*0)));
		printf("Resource record CLASS: %d\n", ntohs(*(uint16_t*)(packet+length+strlen(packet+length)+1+sizeof(uint16_t)*1)));
		printf("Resource record TTL: %d\n", ntohl(*(uint32_t*)(packet+length+strlen(packet+length)+1+sizeof(uint16_t)*2)));
		printf("Resource record RDLENGTH: %d\n", ntohs(*(uint16_t*)(packet+length+strlen(packet+length)+1+sizeof(uint16_t)*4)));
		printf("Resource record RDATA: %d\n", ntohl(*(uint32_t*)(packet+length+strlen(packet+length)+1+sizeof(uint32_t)*5)));
		length += strlen(packet+length)+1 + sizeof(uint16_t)*7;
	}
	printf("packet length: %d\n", length);
}

// TODO: need to debug here to find the right place of IP
int parse_response(char* response, struct addrinfo **res, int packet_length){
	print_serialized_packet(response, packet_length);
	char* QNAME = (char*)(response + sizeof(struct header));
	int offest = packet_length + strlen(QNAME)+1;
	// printf("parse response: %s, %d\n", QNAME, offest);
	char* ip = response + offest + sizeof(uint16_t)*5;
	// printf("%d\n", ntohl(*(uint32_t*)ip));
	((struct sockaddr_in*)(*res)->ai_addr)->sin_addr.s_addr = ntohl(*(uint32_t*)ip);
	int RCODE = ntohl(((struct header*)response)->RCODE);
	return RCODE;
}