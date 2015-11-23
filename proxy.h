#include "common.h"
#include <stdbool.h>

#ifndef _PROXY_H_
#define _PROXY_H_

#define BUF_SIZE 				40960

/* proxy server pair */
typedef struct conn_wrap {
	int client_fd;
	int server_fd;
	char client_buf[BUF_SIZE];
	char server_buf[BUF_SIZE];
	unsigned client_buf_len;
	unsigned server_buf_len;
	/* stat parameters */
	bool all_data_received;
	unsigned long start_time;
	unsigned bitrate;
	unsigned trasmitted_size;
	char chunk_name[MAX_CHUNK_NAME_LEN];
	char server_ip[IP_ADDR_LEN];
	struct conn_wrap * next;
} conn_wrap_t;

#endif
