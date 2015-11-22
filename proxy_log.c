#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define LOG_BUF_SIZE	8192

static FILE * logfile;

void log_init(const char * log_file_name) {
	logfile = fopen(log_file_name, "w");
	/* To ensure that a log of content is not buffered */
	setvbuf(logfile, NULL, _IOLBF, 0);
}

void log_destory() {
	fclose(logfile);
}

void log_error(char * error_msg) {
	fwrite(error_msg, 1, strlen(error_msg), logfile);
}

void log_msg(const char * format, ...) {
	int len;
	char buf[LOG_BUF_SIZE];
	va_list args;
	va_start(args, format);	
	len = vsnprintf(buf, LOG_BUF_SIZE - 1, format, args);
	va_end(args);
	fwrite(buf, 1, len, logfile);
}

/* After each request, it should append the following line to the log:
 * <time> <duration> <tput> <avg-tput> <bitrate> <server-ip> <chunkname>
 */
void log_parameters(unsigned long time, float duration, unsigned now_put,
	unsigned ave_put, unsigned bitrate, const char * server_ip, 
	const char * chunkname)
{
	log_msg("%lu %f %u %u %u %s %s\n", time, duration, now_put, ave_put, 
		bitrate, server_ip, chunkname);
}