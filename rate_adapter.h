#include "common.h"

#ifndef _RATE_ADAPTER_H_
#define	_RATE_ADAPTER_H_

typedef struct server_list {
	char server_ip[IP_ADDR_LEN]
	unsigned bitrate;
	struct server_list * next;
} server_list_t;

typedef struct bitrate_list {
	char name[MAX_CHUNK_NAME_LEN]
	unsigned * bitrates;
	struct bitrate_list * next;
} bitrate_list_t;

unsigned choose_bitrate(const char * server_ip, const char * name);
int adpater_init(double alpha);

#endif
