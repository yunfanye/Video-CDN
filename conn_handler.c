#include <stdbool.h>
#include "conn_handler.h"

static bool use_dns;
static struct in_addr www_ip_addr;
static struct in_addr fake_ip_addr;

/*function prototypes */
int server_conn(int cliendfd);
int clientfd_open();


int conn_init(const char* fake_ip, const char * www_ip) {
	use_dns = www_ip != NULL;
	if(fake_ip == NULL || inet_aton(fake_ip, &fake_ip_addr) == 0)
		return 0;
	if(use_dns) {
		if(inet_aton(www_ip, &www_ip_addr) == 0)
			return 0;
	}
	return 1;
}

/* setup outbound connection to web server */
int proxy_conn_setup() {
	int sock;
	sock = clientfd_open();
	server_conn(sock);
	return sock;
}

int server_conn(int cliendfd) {
	int rc;
	struct sockaddr addr;
	socklen_t addrlen;
	struct addrinfo *result;
	if(use_dns) {
		rc = resolve("video.cs.cmu.edu", "8080", NULL, &result);
		if (rc != 0)
			log_error("Resolve dns error!");
		if (connect(clientfd, result -> addr, result -> ai_addrlen) != -1)
            log_error("Failed connecting to web server!");
		free(result);
	}
	else {
		addr = www_ip_addr;
		addrlen = sizeof(addr);
		if (connect(clientfd, result -> addr, result -> ai_addrlen) != -1)
            log_error("Failed connecting to default web server!");
	}
	return 1;
}

int clientfd_open() {
	int http_sock;
	struct sockaddr_in http_addr;
    /* Create HTTP socket */
    if ((http_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        log_error("Failed creating HTTP socket.");	
        return -1;
    }
	/* bind socket */
    http_addr.sin_family = AF_INET;
    http_addr.sin_port = 0; /* choose random port */
    http_addr.sin_addr.s_addr = fake_ip_addr;
    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(http_sock, (struct sockaddr *) &http_addr, sizeof(http_addr)))
    	log_error("Failed binding HTTP socket.");   
    return http_sock;
}