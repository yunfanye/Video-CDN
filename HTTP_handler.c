
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "HTTP_handler.h"
#include "proxy_log.h"


extern int yyparse (void);
extern void set_parsing_buf(char *buf, size_t siz);
extern char _token[SMALL_BUF_SIZE];
extern char _text[SMALL_BUF_SIZE];

enum parse_state {
	STATE_START = 0,
	STATE_CR,
	STATE_CRLF,
	STATE_CRLFCR,
	STATE_CRLFCRLF
};

int extract_video_name(const char * request, int buf_size, char * out_buf) {
	int index = 0;
	const char * buf = request;
	char work_buf[SMALL_BUF_SIZE];
	char method[SMALL_BUF_SIZE];
	char prot[SMALL_BUF_SIZE];
	enum parse_state state = STATE_START;	
	
	while(index < buf_size && state != STATE_CRLFCRLF) {
		char expected = 0;
		switch(state) {
			case STATE_START:
				expected = '\r';
				break;
			case STATE_CRLF:
				memcpy(work_buf, buf, index - 2);
				work_buf[index - 1] = '\0';
				log_msg("request line: %s\n", work_buf);
				if(sscanf(work_buf, "%s %s %s", method, out_buf, prot) != 3)
					return 0;
				return 1;
			case STATE_CR:
			case STATE_CRLFCR:
				expected = '\n';
				break;
			default:
				state = STATE_START;
				continue;
		}
		if(buf[index] == expected)
			state++;
		else
			state = STATE_START;
		index++;
	}
	return 0;
}

/* change old uri to new uri; add or change header to connection: close */
int process_clinet_request(char * request, unsigned int * old_buf_size,
	const char * new_uri) 
{
	int buf_size = *old_buf_size;
	int index = 0, last_index = 0;
	char * buf = request;
	char work_buf[SMALL_BUF_SIZE];
	char add_new_buf[SMALL_BUF_SIZE];
	char old_uri[SMALL_BUF_SIZE];
	char method[SMALL_BUF_SIZE];
	char prot[SMALL_BUF_SIZE];
	enum parse_state state = STATE_START;
	bool had_request_line = false;
	int new_len;
	
	while(index < buf_size && state != STATE_CRLFCRLF) {
		char expected = 0;
		switch(state) {
			case STATE_START:
				expected = '\r';
				break;
			case STATE_CRLF:
				/* parse one line */
				if(had_request_line) {
					set_parsing_buf(buf + last_index, index - last_index - 2);	
					if(yyparse()) {
						log_error("Parse headers error!");
					}
					else {
						/* remove the head */
						if(!strcmp(_token, "Connection")) {
							memmove(request + last_index, request + index, buf_size - index);
							buf_size -= index - last_index;
							index = last_index;
						}
					}
				} else {
					memcpy(work_buf, buf, index - 2);
					work_buf[index - 1] = '\0';
					if(sscanf(work_buf, "%s %s %s", method, old_uri, prot) != 3)
						log_error("Parse request line error!");
					/* change to new request line */
					new_len = snprintf(add_new_buf, SMALL_BUF_SIZE, "%s %s %s\r\nConnection: Close\r\n", 
						method, new_uri, prot);
					/* make space */
					memmove(request + last_index + new_len, request + index, 
						buf_size - index);
					buf_size += last_index + new_len - index;
					/* insert new line */
					memcpy(request + last_index, add_new_buf, new_len);
					index = last_index + new_len;
					had_request_line = true;
				}
				last_index = index;
				expected = '\r';
				break;
			case STATE_CR:
			case STATE_CRLFCR:
				expected = '\n';
				break;
			default:
				state = STATE_START;
				continue;
		}

		if(buf[index] == expected)
			state++;
		else
			state = STATE_START;
		index++;
	}
	
	*old_buf_size = buf_size;
	if(state == STATE_CRLFCRLF) {
		return 1;
	}
	return 0;
}

/* change request URI to /path/to/video/500Seg2-Frag3 */
int change_URI(char * old_uri, int bitrate) {
	char * target;
	int str_len;
	int index;
	char buf[SMALL_BUF_SIZE];
	/* if it is xxxx.f4m, change it to xxxx_nolist.f4m */
	if((target = strcasestr(old_uri, ".f4m")) != NULL) {
		strcpy(target, "_nolist.f4m");
	}
	else if(strcasestr(old_uri, ".html") != NULL) {
		return 1;
	}
	else {
		str_len = strlen(old_uri);
		index = str_len - 1;
		while(old_uri[index] != '/')
			index--;
		if((target = strcasestr(old_uri + index, "Seg")) == NULL) {
			log_error("Cannot find seg!");
			return 0;
		}
		old_uri[index] = '\0';
		snprintf(buf, SMALL_BUF_SIZE, "%s/%d%s", old_uri, bitrate, target);
		strcpy(old_uri, buf);
	}
	return 1;
}