#ifndef __UTIL__
#define __UTIL__

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#include <arpa/inet.h>
#include <string.h>

#define MAX_BUFFER 8192

struct header{
	uint16_t ID;
	
	uint8_t RD:1;
	uint8_t TC:1;
	uint8_t AA:1;
	uint8_t Opcode:4;
	uint8_t QR:1;

	uint8_t RCODE:4;
	uint8_t Z:3;
	uint8_t RA:1;

	uint16_t QDCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
	
};

struct question{
	char* QNAME;
	uint16_t QTYPE;
	uint16_t QCLASS;
};

// Shared by ANSWER, AUTHORITY and ADDITIONAL sections
struct resource_record{
	char* NAME;
	uint16_t TYPE;
	uint16_t CLASS;
	uint32_t TTL;
	uint16_t RDLENGTH;
	uint32_t RDATA;
};

struct packet{
	struct header* header;
	struct question* question;
	struct resource_record* resource_record;
};

// void parse_response(char* response, struct addrinfo **res, int packet_length);
void serialize(struct packet* packet, char* data, int* length);
char* make_error_response_packet(char* input_buffer, int* response_length);
char* make_response_packet(char* input_buffer, char* dest_addr, int* response_length);
void free_packet(struct packet* packet);
struct packet* make_query_packet(const char* node);
void make_addrinfo(struct addrinfo** res);
void print_packet(struct packet* packet);
void print_serialized_packet(char* packet, int packet_length);
int parse_response(char* response, struct addrinfo **res, int packet_length);
void convertName(char* name, const char* const_src);

#endif