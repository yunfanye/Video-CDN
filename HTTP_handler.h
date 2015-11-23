#ifndef _HTTP_HANDLER_H_
#define _HTTP_HANDLER_H_

int extract_video_name(const char * request, int buf_size, char * out_buf);

int process_clinet_request(char * request, unsigned int * old_buf_size, 
	const char * new_uri);

int change_URI(char * old_uri, int bitrate);

#endif
