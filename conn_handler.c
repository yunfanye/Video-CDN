#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "conn_handler.h"
#include "mydns.h"
#include "time_util.h"
#include "proxy_log.h"

static bool use_dns;
static in_addr_t www_ip_addr;
static in_addr_t fake_ip_addr;

/*function prototypes */
int server_conn(int cliendfd, char * server_ip);
int clientfd_open();


int conn_init(const char* fake_ip, const char * www_ip) {
	use_dns = www_ip == NULL;
	log_msg("fake ip: %s, www ip %s\n", fake_ip, www_ip);
	if(fake_ip == NULL || (fake_ip_addr = inet_addr(fake_ip)) == INADDR_NONE)
		return 0;
	if(!use_dns) {
		if((www_ip_addr = inet_addr(www_ip)) == INADDR_NONE)
			return 0;
	}
	return 1;
}

/* setup outbound connection to web server */
int proxy_conn_setup(char * server_ip) {
	int sock;
	sock = clientfd_open();
	log_msg("open clientfd: %d\n", sock);
	server_conn(sock, server_ip);
	log_msg("connected to server\n");
	return sock;
}

int server_conn(int clientfd, char * server_ip) {
	int rc;
	struct sockaddr_in addr;
	socklen_t addrlen;
	struct addrinfo *result;
	char * ip_str;
	if(use_dns) {
		rc = resolve("video.cs.cmu.edu", "8080", NULL, &result);
		if (rc != 0)
			log_error("Resolve dns error!");
		if (connect(clientfd, result -> ai_addr, result -> ai_addrlen) != -1)
            log_error("Failed connecting to web server!");
		ip_str = inet_ntoa(
			((struct sockaddr_in *)(result -> ai_addr)) -> sin_addr);
		free(result);
	}
	else {

		addr.sin_family = AF_INET;
		addr.sin_port = 80;
		addr.sin_addr.s_addr = www_ip_addr;
		addrlen = sizeof(addr);
		if (connect(clientfd, (struct sockaddr *) &addr, addrlen) != -1)
            log_error("Failed connecting to default web server!");
        ip_str = inet_ntoa(addr.sin_addr); /* statically allocated */
        log_msg("connect to www_ip: %s\n", ip_str);
	}
	strcpy(server_ip, ip_str);
	return 1;
}

int clientfd_open() {
	int http_sock;
	struct sockaddr_in http_addr;
    /* Create HTTP socket */
    if ((http_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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
