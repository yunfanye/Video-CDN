#ifndef _PROXY_LOG_H_
#define _PROXY_LOG_H_

void log_init(const char * log_filename);
void log_destory();
void log_error(char * error_msg);
void log_msg(const char * format, ...);

void log_parameters(unsigned long time, float duration, unsigned now_put,
	unsigned ave_put, unsigned bitrate, char * server_ip, char * chunkname);

#endif