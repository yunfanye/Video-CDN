
#define _GNU_SOURCE

#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mydns.h"
#include "conn_handler.h"
#include "proxy.h"
#include "rate_adapter.h"
#include "time_util.h"
#include "proxy_log.h"
#include "HTTP_handler.h"
#include "f4m_parser.h"
#include "name_util.h"

#define MAX(x, y) ((x)>(y)?(x):(y))

/* global variables */
static int _http_sock;

static int _port;

/* function prototypes */
int parse_command_line(int argc, char * argv[]);
void mHTTP_init(int http_port);
int handle_conn(conn_wrap_t * node, fd_set * readset, fd_set * writeset);
conn_wrap_t * add_linkedlist_node(conn_wrap_t * head, int client_fd);
conn_wrap_t * accept_new_request(conn_wrap_t * head, int http_sock);
conn_wrap_t * remove_linkedlist_node(conn_wrap_t * head, conn_wrap_t ** node);
ssize_t mRecv(int sockfd, void * buf, size_t readlen);
ssize_t mSend(int sockfd, const void * buf, size_t writelen);
int close_socket(int sock);

conn_wrap_t * self_req_head;

int main(int argc, char* argv[]) {
	fd_set readset, writeset;
	int nfds;
	struct timeval timeout;
	conn_wrap_t * head = NULL;
	conn_wrap_t * loop_node;
	unsigned * bitrates;

	/* check and parse command line arguments */
	if(argc < 7)
		return EXIT_FAILURE;
	parse_command_line(argc, argv);

	while (1) {
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		/* init fd sets */
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_SET(_http_sock, &readset);   	
		nfds = _http_sock;
		/* client request forwarding */
		loop_node = head;
		while(loop_node != NULL) {
			if(loop_node -> server_buf_len > 0)
				FD_SET(loop_node -> client_fd, &writeset);
			if(loop_node -> client_buf_len > 0)
				FD_SET(loop_node -> server_fd, &writeset);
			if(loop_node -> client_buf_len < BUF_SIZE)
				FD_SET(loop_node -> client_fd, &readset);
			if(loop_node -> server_buf_len < BUF_SIZE)
				FD_SET(loop_node -> server_fd, &readset);
			nfds = MAX(nfds, loop_node -> client_fd);
			nfds = MAX(nfds, loop_node -> server_fd);
			loop_node = loop_node -> next;
		}
		/* self initiated requests */
		loop_node = self_req_head;
		while(loop_node != NULL) {
			if(loop_node -> client_buf_len > 0)
				FD_SET(loop_node -> server_fd, &writeset);
			if(loop_node -> server_buf_len < BUF_SIZE)
				FD_SET(loop_node -> server_fd, &readset);
			nfds = MAX(nfds, loop_node -> server_fd);
			loop_node = loop_node -> next;
		}
		nfds++; /* plus one! */
		log_msg("start selecting! nfds: %d\n", nfds);
		if(select(nfds, &readset, &writeset, NULL, &timeout) >= 0) {

			if(FD_ISSET(_http_sock, &readset)) {			
				head = accept_new_request(head, _http_sock);
				log_msg("new request, head: %p\n", head);
			}

			loop_node = head;
			while(loop_node != NULL) {
				log_msg("handle client initiated request!\n");
				if(handle_conn(loop_node, &readset, &writeset) == 0)
					head = remove_linkedlist_node(head, &loop_node);
				else
					loop_node = loop_node -> next;
			}

			/* self initiated req to get f4m */
			loop_node = self_req_head;
			while(loop_node != NULL) {
				if(loop_node -> all_data_received) {
					loop_node -> server_buf[loop_node -> server_buf_len] ='\0';
					log_msg("client %d, server %d, client buf: %s, f4m doc: %p, %d\ncontent: %s\n", 
						loop_node -> client_fd, loop_node -> server_fd, loop_node -> client_buf,
						loop_node -> server_buf, loop_node -> server_buf_len, loop_node -> server_buf);
					bitrates = extract_bitrate_list(loop_node -> server_buf,
						loop_node -> server_buf_len);
					set_bitrate_list(loop_node -> chunk_name, bitrates);
					self_req_head = remove_linkedlist_node(self_req_head, &loop_node);
				}
				else {
					log_msg("handle server initiated request!\n");
					handle_conn(loop_node, &readset, &writeset);
					loop_node = loop_node -> next;
				}
			}
		}
	}
	
	return 0;
}

int generate_request(conn_wrap_t * head, const char * chunk_name) {
	conn_wrap_t * node;
	char name[SMALL_BUF_SIZE];
	char buf[SMALL_BUF_SIZE];
	get_videoname_from_chunkname(chunk_name, name);
	/* generate f4m request */
	self_req_head = add_linkedlist_node(self_req_head, -1);
	node = self_req_head;
	/* TODO: URI format */
	snprintf(buf, SMALL_BUF_SIZE, "GET /vod/big_buck_bunny.f4m HTTP/1.1\r\n"
		"Host: 4.0.0.1:%d\r\nConnection: close\r\nAccept: */*\r\n"
		"Accept-Encoding: gzip, deflate, sdch\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"
		" (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n\r\n",
		_port);

	strcpy(node -> client_buf, buf);
	strcpy(node -> chunk_name, chunk_name);
	node -> client_buf_len = strlen(buf);
	log_msg("get f4m from web server, len %d\n%s", node -> client_buf_len, buf);
	return 1;
}

int handle_conn(conn_wrap_t * node, fd_set * readset, fd_set * writeset) {
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
	server_buf = node -> server_buf;
	log_msg("handle_conn: node %p, server buf %d, client buf %d\n", node, server_len, client_len);
	/* begin interact with client */
	/* send video data to client */
	if(client != -1  && FD_ISSET(client, writeset) && server_len > 0) {
		log_msg("send data to client\n", server_len);
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
	/* recv request from client */
	if(client != -1 && FD_ISSET(client, readset) && client_len < BUF_SIZE) {
		readlen = BUF_SIZE - client_len;
		log_msg("recv data from client, client %d old client len %d\n", client, client_len);
		if((readret = mRecv(client, client_buf + client_len, readlen)) > 0) {
			client_len += readret;
			node -> client_buf_len = client_len;
			log_msg("to extract URI\n");
			/* Process request */
			extract_video_name(client_buf, node -> client_buf_len, 
				node -> chunk_name);
			log_msg("client request %s\n", node -> chunk_name);
			if(strcasestr(node -> chunk_name, ".f4m") != NULL ||
				strcasestr(node -> chunk_name, ".html") != NULL ||
				strcasestr(node -> chunk_name, ".ico") != NULL ||
				strcasestr(node -> chunk_name, ".swf") != NULL ||
				strcasestr(node -> chunk_name, ".js") != NULL)
			{
				/* skip .f4m and .html */
				node -> bitrate = 0;
			}
			else {
				node -> bitrate = choose_bitrate(node -> server_ip, 
					node -> chunk_name);
			}
			log_msg("chose bitrate %d\n", node -> bitrate);
			if(node -> bitrate == -1) {
				/* try to get f4m */
				generate_request(self_req_head, node -> chunk_name);
				return 1;			
			}
		}
		else {
			/* if interrupted, read next time; otherwise, release 
			 * it. Or if recv returns 0, which means peer shut down
			 * the connection, close the socket */
			if(readret == 0 || (readret == -1 && errno != EINTR))
				return 0;
		}
	}

	if(node -> all_data_received) {
		/* tell client no more data */
		if(client != -1)
			shutdown(client, SHUT_WR);
		return server_len == 1;
	}

	/* begin interact with web server */
	/* send request to web server */
	if(FD_ISSET(server, writeset) && client_len > 0) {
		/* block until f4m file is acquired */
		log_msg("Send request to web server %d\n", client_len);
		if(node -> bitrate == -1) {
			node -> bitrate = choose_bitrate(node -> server_ip, 
				node -> chunk_name);
			log_msg("new bitrate chosen: %d\n", node -> bitrate);
			return 1;
		}
		if(client != -1) {
			/* if it is client initiated request,
			 * process request and modify request before sending */
			change_URI(node -> chunk_name, node -> bitrate);
			process_clinet_request(client_buf, &client_len, 
				node -> chunk_name);
		}
		node -> client_buf_len = client_len;

		client_buf[client_len] = '\0';
		log_msg("Detailed Send %p\n%s to web server %d\n", client_buf, client_buf, client_len);

		if ((writeret = mSend(server, client_buf, client_len)) != client_len) {
			/* if some bytes have been sent */
			log_msg("Send not completed! sent %d\n", client_len);
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
			log_msg("Send completed!\n");
			node -> client_buf_len = 0;
		}
	}
	/* recv video data from web server */
	if(FD_ISSET(server, readset) && server_len < BUF_SIZE) {
		readlen = BUF_SIZE - server_len;
		log_msg("recv data from web server: server %d, old len %d\n", server, server_len);
		if((readret = mRecv(server, server_buf + server_len, readlen)) > 0) {
			server_len += readret;
			node -> server_buf_len = server_len;
			node -> trasmitted_size += readret;
		}
		else {
			/* if interrupted, read next time; otherwise, release 
			 * it. Or if recv returns 0, which means peer shut down
			 * the connection, close the socket */
			if(readret == 0) {
				/* finish */
				node -> all_data_received = true;
				estimate_tp(node -> start_time, node -> trasmitted_size, 
					node -> bitrate, node -> server_ip, node -> chunk_name);
				close_socket(server);
				node -> server_fd = -1;
			}
			else if(readret == -1 && errno != EINTR)
				return 1;
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

conn_wrap_t * add_linkedlist_node(conn_wrap_t * head, int client_fd) {				
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
	/* output server_ip */
	head -> server_fd = proxy_conn_setup(head -> server_ip);
	log_msg("setup proxy outbound conn: %s\n", head -> server_ip);
	head -> client_buf_len = 0;
	head -> server_buf_len = 0;
	head -> trasmitted_size = 0;
	head -> all_data_received = false;
	head -> start_time = milli_time();
	return head;
}

conn_wrap_t * remove_linkedlist_node(conn_wrap_t * head, 
	conn_wrap_t ** node_ptr) 
{
	conn_wrap_t * returned_head = head;
	conn_wrap_t * node = *node_ptr;
	if(head == node) {
		head = head -> next;
		*node_ptr = head;
		returned_head = head;
	}
	else {
		while(head != NULL && head -> next != node)
			head = head -> next;
		head -> next = node -> next;
		*node_ptr = head -> next;		
	}
	/* close conns */
	if(node -> client_fd != -1)
		close_socket(node -> client_fd);
	if(node -> server_fd != -1)
		close_socket(node -> server_fd);
	free(node);
	return returned_head;
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
    if ((_http_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        log_error("Failed creating HTTP socket.");	
	/* bind socket */
    http_addr.sin_family = AF_INET;
    http_addr.sin_port = htons(http_port);
    http_addr.sin_addr.s_addr = INADDR_ANY;
    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(_http_sock, (struct sockaddr *) &http_addr, sizeof(http_addr))) {
    	log_error("Failed binding HTTP socket.");   
    	exit(EXIT_FAILURE);
    }
	/* listen to the sock */
    if (listen(_http_sock, 50)) 
    	log_error("Error listening on HTTP socket.");
    log_msg("start listening on port %d\n", http_port);
    _port = http_port;
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
	log_msg("Send! sock %d, buf %p, len %d\n", sockfd, buf, writelen);
	if((writeret = send(sockfd, buf, writelen, 0)) < 0) {
		log_msg("Send error!\n");
		log_error("Send error!");
	}
	log_msg("Send success: %d!\n", writeret);
	return writeret;
}

int close_socket(int sock)
{
    if (close(sock))
    {
        log_error("Failed closing socket.\n");
        return 1;
    }
    return 0;
}
