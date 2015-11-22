#include "common.h"

#ifndef _RATE_ADAPTER_H_
#define	_RATE_ADAPTER_H_

typedef struct server_list {
	char server_ip[IP_ADDR_LEN];
	unsigned bitrate;
	struct server_list * next;
} server_list_t;

typedef struct bitrate_list {
	char name[MAX_CHUNK_NAME_LEN];
	unsigned * bitrates;
	struct bitrate_list * next;
} bitrate_list_t;

int choose_bitrate(const char * server_ip, const char * name);
int adapter_init(double alpha);
/* call after chunk is finished */
int estimate_tp(unsigned long start_time, unsigned transmitted_size, 
	unsigned bitrate, const char * server_ip, const char * chunk_name);

#endif
