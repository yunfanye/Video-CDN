#include "common.h"
#include "time_util.h"
#include "rate_adapter.h"
#include "proxy_log.h"
#include "name_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RATIO		1.5

static double alpha;

static server_list_t * server_list;

static bitrate_list_t * bitrate_list;

int set_bitrate_list(const char * chunk_name, unsigned * list) {
	char name[SMALL_BUF_SIZE];
	bitrate_list_t * node = bitrate_list;
	get_videoname_from_chunkname(chunk_name, name);
	log_msg("set bitrate list: %s\n", chunk_name);
	while(node != NULL && node -> next !=  NULL) {
		if(!strcmp(node -> name, name))
			return 1;
		node = node -> next;
	}
	if(node != NULL && !strcmp(node -> name, name))
		return 1;
	else {
		log_msg("set new bitrate list: %s\n", name);
		if(node == NULL) {
			node = malloc(sizeof(bitrate_list_t));
			bitrate_list = node;
			log_msg("bitrate list header modified\n");
		}
		else {
			node -> next = malloc(sizeof(bitrate_list_t));
			node = node -> next;
		}
		node -> next = NULL;
		strcpy(node -> name, name);
		node -> bitrates = list; 
		return 0;
	}
}

unsigned * get_bitrate_list(const char * chunk_name) {
	char name[SMALL_BUF_SIZE];
	bitrate_list_t * node = bitrate_list;
	get_videoname_from_chunkname(chunk_name, name);
	log_msg("node: %p, get bitrate list: %s\n", node, name);
	while(node !=  NULL) {
		if(!strcmp(node -> name, name))
			return node ->  bitrates;
		node = node -> next;
	}
	/* not found, try to get f4m file */
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

int set_server_bitrate(const char * server_ip, unsigned bitrate) {
	server_list_t * node = server_list;
	while(node !=  NULL) {
		if(!strcmp(node -> server_ip, server_ip)) {
			node -> bitrate = bitrate;
			return 1;
		}
		node = node -> next;
	}
	return 0;
}

unsigned get_server_bitrate(const char * server_ip) {
	server_list_t * node = server_list;
	while(node !=  NULL) {
		if(!strcmp(node -> server_ip, server_ip))
			return node ->  bitrate;
		node = node -> next;
	}
	/* not found, add server to list */
	add_to_server_list(server_ip);
	return 0;
}

int choose_bitrate(const char * server_ip, const char * name) {
	int i;
	unsigned * bitrates = get_bitrate_list(name); /* 0 terminates array */
	unsigned now_bitrate = get_server_bitrate(server_ip);
	unsigned result;
	if(bitrates == NULL)
		return -1;
	/* by default choose lowest quality */
	result = bitrates[0];
	if(now_bitrate > 0 && result > 0) {
		i = 1;
		printf("choose_bitrate: %s, %s, %d, %d, %d\n", server_ip, name, i, now_bitrate, result);
		/* choose maximum bit rate, 1.5x which less than now_bitrate */
		while(bitrates[i] != 0 && bitrates[i] * RATIO < now_bitrate) {
			result = bitrates[i];
			printf("inner choose_bitrate: %s, %s, %d, %d, %d, %d\n", server_ip, name, i, now_bitrate, result, bitrates[i+1]);
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
	ave_put = alpha * now_put + (1.0 - alpha) * ave_put;
	// printf("estimate_tp: %d, %d, %d, %f, %f\n", ave_put, now_put, transmitted_size, alpha, duration);
	set_server_bitrate(server_ip, ave_put);
	log_parameters(end_time/1000, duration, now_put/1000, ave_put/1000, bitrate,
		server_ip, chunk_name);
	return 1;
}
