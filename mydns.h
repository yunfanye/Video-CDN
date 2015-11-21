#ifndef __MYDNS__
#define __MYDNS__

#include <netdb.h>

#define MAX_BUFFER 8192

// Global variables used by DNS servers
int dns_socket;
struct sockaddr_in dns_server_addr;

// Helper strcuts
struct header{
	uint16_t ID;

	uint16_t QR:1;
	uint16_t Opcode:4;
	uint16_t AA:1;
	uint16_t TC:1;
	uint16_t RD:1;
	uint16_t RA:1;
	uint16_t Z:3;
	uint16_t RCODE:4;

	uint16_t QDCOUNT;
	uint16_t ANCOUNT;
	uint16_t NSCOUNT;
	uint16_t ARCOUNT;
	
};

struct question{
	uint16_t QNAME;
	uint16_t QTYPE;
	uint16_t QCLASS;
};

// Shared by ANSWER, AUTHORITY and ADDITIONAL sections
struct resource_record{
	uint16_t NAME;
	uint16_t TYPE;
	uint16_t CLASS;
	uint16_t TTL;
	uint16_t RDLENGTH;
	uint16_t RDATA;
};

struct packet{
	struct header* header;
	struct question* question;
	struct resource_record* resource_record;
};

/**
 * Initialize your client DNS library with the IP address and port number of
 * your DNS server.
 *
 * @param  dns_ip  The IP address of the DNS server.
 * @param  dns_port  The port number of the DNS server.
 *
 * @return 0 on success, -1 otherwise
 */
int init_mydns(const char *dns_ip, unsigned int dns_port);


/**
 * Resolve a DNS name using your custom DNS server.
 *
 * Whenever your proxy needs to open a connection to a web server, it calls
 * resolve() as follows:
 *
 * struct addrinfo *result;
 * int rc = resolve("video.cs.cmu.edu", "8080", null, &result);
 * if (rc != 0) {
 *     // handle error
 * }
 * // connect to address in result
 * free(result);
 *
 *
 * @param  node  The hostname to resolve.
 * @param  service  The desired port number as a string.
 * @param  hints  Should be null. resolve() ignores this parameter.
 * @param  res  The result. resolve() should allocate a struct addrinfo, which
 * the caller is responsible for freeing.
 *
 * @return 0 on success, -1 otherwise
 */

int resolve(const char *node, const char *service, 
            const struct addrinfo *hints, struct addrinfo **res);

#endif