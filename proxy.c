

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mydns.h"
#include "conn_handler.h"
#include "proxy.h"
#include "rate_adapter.h"
#include "time_util.h"

/* global variables */
static int http_sock;

/* function prototypes */
int parse_command_line(int argc, char * argv[]);
void mHTTP_init(int http_port);
int handle_conn(conn_wrap_t * node, fd_set readset, fd_set writeset);
int add_linkedlist_node(conn_wrap_t * head, int client_fd);
conn_wrap_t * accept_new_request(conn_wrap_t * head, int http_sock);
ssize_t mRecv(int sockfd, void * buf, size_t readlen);
ssize_t mSend(int sockfd, const void * buf, size_t writelen);


int main(int argc, char* argv[]) {
	fd_set readset, writeset;
	int nfds;
	struct timeval timeout;
	conn_wrap_t * head = NULL;
	conn_wrap_t * loop_node;
	/* check and parse command line arguments */
	if(argc < 7)
		return EXIT_FAILURE;
	parse_command_line(argc, argv);

	while (1) {
		/* init fd sets */
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_SET(http_sock, &readset);   	
		nfds = http_sock;

		loop_node = head;
		while(loop_node != NULL) {
			if(loop_node -> server_buf_len > 0)
				FD_SET(client, writeset);
			if(loop_node -> client_buf_len > 0)
				FD_SET(server, writeset);
			if(loop_node -> client_buf_len < BUF_SIZE)
				FD_SET(client, readset);
			if(loop_node -> server_buf_len < BUF_SIZE)
				FD_SET(server, readset);
			nfds = MAX(nfds, loop_node -> client_fd);
			nfds = MAX(nfds, loop_node -> server_fd);
			loop_node = loop_node -> next;
		} 

		if((selectRet = select(nfds, &readset, &writeset, NULL, 
			&timeout)) >= 0) {

			if(FD_ISSET(http_sock, &readset)) {
				head = accept_new_request(head, http_sock);
			}

			loop_node = head;
			while(loop_node != NULL) {
				if(handle_conn(loop_node, readset, writeset) == 0) {
					/* TODO: remove this conn */
				}
				loop_node = loop_node -> next;
			}
		}
	}
	
	return 0;
}

int handle_conn(conn_wrap_t * node, fd_set readset, fd_set writeset) {
	int client, server;
	unsigned client_len, server_len;
	char * client_buf, * server_buf;
	int readret;
	int writeret;
	unsigned readlen;
	client = node -> client_fd;
	server = node -> server_fd;
	client_len = node -> client_buf_len;
	server_len = node -> server_buf_len;
	client_buf = node -> client_buf;
	server_buf = node -> client_buf;

	if(FD_ISSET(client, writeset) && server_len > 0) {
		if ((writeret = mSend(client, server_buf, server_len)) != server_len) {
			/* if some bytes have been sent */
			if(writeret != -1) {
				server_len -= writeret;
				/* src and dest may overlap, so use memmove instead
				 * of memcpy */
				memmove(server_buf, server_buf + writeret, server_len);
				node -> server_buf_len = server_len;
			}
			else {
				/* failure due to an interruption, send should restart; otherwise,
				 * the connection is no longer effective, remove it;
				 */
				if(errno != EINTR)
					return 0;
			}
		}
		else {
			node -> server_buf_len = 0;
		}
	}

	if(FD_ISSET(client, readset) && client_len < BUF_SIZE) {
		readlen = BUF_SIZE - client_len;
		if((readret = mRecv(client, client_buf + client_len, readlen)) > 0) {
			client_len += readret;
			node -> client_buf_len = client_len; 
		}
		else {
			/* if interrupted, read next time; otherwise, release 
			 * it. Or if recv returns 0, which means peer shut down
			 * the connection, close the socket */
			if(readret == 0 || (readret == -1 && errno != EINTR))
				return 0;
		}
	}

	if(all_data_received) {
		/* tell client no more data */
		shutdown(client, SHUT_WR);
		return server_len == 1;
	}

	/* interact with web server */
	if(FD_ISSET(server, writeset) && client_len > 0) {
		if ((writeret = mSend(server, client_buf, client_len)) != client_len) {
			/* if some bytes have been sent */
			if(writeret != -1) {
				client_len -= writeret;
				/* src and dest may overlap, so use memmove instead
				 * of memcpy */
				memmove(client_buf, client_buf + writeret, client_len);
				node -> client_buf_len = client_len;
			}
			else {
				/* failure due to an interruption, send should restart;
				 * otherwise, the connection is no longer effective, remove it;
				 */
				if(errno != EINTR)
					return 0;
			}
		}
		else {
			node -> client_buf_len = 0;
		}
	}
	if(FD_ISSET(server, readset) && server_len < BUF_SIZE) {
		readlen = BUF_SIZE - server_len;
		if((readret = mRecv(server, server_buf + server_len, readlen)) > 0) {
			server_len += readret;
			node -> server_buf_len = server_len;
			head -> trasmitted_size += readret;
		}
		else {
			/* if interrupted, read next time; otherwise, release 
			 * it. Or if recv returns 0, which means peer shut down
			 * the connection, close the socket */
			if(readret == 0) {
				/* finish */
				all_data_received = true;
				estimate_tp(node -> start_time, node -> trasmitted_size, 
					node -> server_ip, node -> chunk_name);
			}
			else if(readret == -1 && errno != EINTR)
				return 0;
		}		
	}
	return 1;
}

conn_wrap_t * accept_new_request(conn_wrap_t * head, int http_sock) {
    socklen_t cli_size;
    struct sockaddr_in cli_addr;
    int client_sock;
	/* establish new client socket */
	cli_size = sizeof(cli_addr);
	if ((client_sock = accept(http_sock, 
		(struct sockaddr *) &cli_addr, &cli_size)) != -1) {	
		/* add new client fd to read list */
		head = add_linkedlist_node(head, client_sock);
		return head;
	}
	log_error("Accept new connection error!");
	return head;
}

int add_linkedlist_node(conn_wrap_t * head, int client_fd) {				
	conn_wrap_t * tmp;
	if(!head) {															
		head = malloc(sizeof(conn_wrap_t));							
		head -> next = NULL;											
	}																	
	else {																
		tmp = head;												
		head = malloc(sizeof(conn_wrap_t));							
		head -> next = tmp;										
	}																	
	head -> client_fd = client_fd;
	head -> server_fd = proxy_conn_setup(head -> server_ip);
	head -> client_buf_len = 0;
	head -> server_buf_len = 0;
	head -> trasmitted_size = 0;
	head -> all_data_received = false;
	head -> bitrate = choose_bitrate(head -> server_ip);
	return head;
}

/* ./proxy <log> <alpha> <listen-port> <fake-ip>
 * <dns-ip> <dns-port> [<www-ip>] */
int parse_command_line(int argc, char * argv[]) {
	char * dns_ip;
	unsigned dns_port;
	char * log_filename;
	int listen_port;
	char * fake_ip;
	char * www_ip = NULL;
	double alpha;	
	/* www_ip is specified */
	if(argc == 8)
		www_ip = argv[7];
	log_filename = argv[1];
	alpha = strtod(argv[2], NULL);
	listen_port = atoi(argv[3]);
	fake_ip = argv[4];
	dns_ip = argv[5];
	dns_port = atoi(argv[6]);

	log_init(log_filename);
	init_mydns(dns_ip, dns_port);
	mHTTP_init(listen_port);
	conn_init(fake_ip, www_ip);
	adapter_init(alpha);
	return 0;
}

/* init browser listen socket */
void mHTTP_init(int http_port) {
	struct sockaddr_in http_addr;
    /* Create HTTP socket */
    if ((http_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        log_error("Failed creating HTTP socket.");	
	/* bind socket */
    http_addr.sin_family = AF_INET;
    http_addr.sin_port = htons(http_port);
    http_addr.sin_addr.s_addr = INADDR_ANY;
    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(http_sock, (struct sockaddr *) &http_addr, sizeof(http_addr)))
    	log_error("Failed binding HTTP socket.");   
	/* listen to the sock */
    if (listen(http_sock, 50)) 
    	log_error("Error listening on HTTP socket.");
}

ssize_t mRecv(int sockfd, void * buf, size_t readlen) {
	int readret;
	if((readret = recv(sockfd, buf, readlen, 0)) < 0) {
		log_error("Recv error!");
	}
	return readret;
}

ssize_t mSend(int sockfd, const void * buf, size_t writelen) {
	int writeret;
	if((writeret = send(sockfd, buf, writelen, 0)) < 0) {
		error_log("Send error!\n");
	}
	return writeret;
}