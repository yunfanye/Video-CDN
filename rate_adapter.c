#include "time_util.h"
#include "rate_adapter.h"
#include "proxy_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RATIO		1.5

static double alpha;

static server_list_t * server_list;

static bitrate_list_t * bitrate_list;

unsigned * get_bitrate_list(const char * name) {
	bitrate_list_t * node = bitrate_list;
	while(node !=  NULL) {
		if(!strcmp(node -> name, name))
			return node ->  bitrates;
	}
	/* TODO: not found, try to get f4m file */
	return NULL;
}

void add_to_server_list(const char * server_ip) {
	server_list_t * new_node = malloc(sizeof(server_list_t));
	strcpy(new_node -> server_ip, server_ip);
	new_node -> bitrate = 0;
	if(!server_list) {
		server_list = new_node;
		new_node -> next = NULL;
	}
	else {
		new_node -> next = server_list;
		server_list = new_node;
	}
}

unsigned get_server_bitrate(const char * server_ip) {
	server_list_t * node = server_list;
	while(node !=  NULL) {
		if(!strcmp(node -> server_ip, server_ip))
			return node ->  bitrate;
	}
	/* not found, add server to list */
	add_to_server_list(server_ip);
	return 0;
}

unsigned choose_bitrate(const char * server_ip, const char * name) {
	int i;
	unsigned * bitrates = get_bitrate_list(name); /* 0 terminates array */
	unsigned now_bitrate = get_server_bitrate(server_ip);
	unsigned result;
	/* by default choose lowest quality */
	result = bitrates[0];
	if(now_bitrate > 0 && result > 0) {
		i = 1;
		/* choose maximum bit rate, 1.5x which less than now_bitrate */
		while(bitrates[i] != 0 && bitrates[i] * RATIO < now_bitrate) {
			result = bitrates[i];
			i++;
		}
	}
	return result;
}

int adapter_init(double alpha_in) {
	if(alpha_in > 1) {
		alpha = 0.5;
		return 0;
	}
	alpha = alpha_in;
	server_list = NULL;
	bitrate_list = NULL;
	return 1;
}

int estimate_tp(unsigned long start_time, unsigned transmitted_size, 
	unsigned bitrate, const char * server_ip, const char * chunk_name)
{
	unsigned ave_put;
	unsigned long end_time = milli_time();
	float duration = end_time - start_time;
	unsigned now_put;
	ave_put = get_server_bitrate(server_ip);
	now_put = (double)transmitted_size * 1000.0/ (double)duration;
	/* estimate */
	ave_put = alpha * now_put + (1 - alpha) * ave_put; 
	log_parameters(end_time/1000, duration, now_put/1000, ave_put/1000, bitrate,
		server_ip, chunk_name);
	return 1;
}
