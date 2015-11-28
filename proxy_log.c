#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define LOG_BUF_SIZE	8192

static FILE * logfile;
static FILE * logfile_debug;

void log_init(const char * log_file_name) {
	logfile = fopen(log_file_name, "w");
	logfile_debug = fopen("debug.log", "w");
	/* To ensure that a log of content is not buffered */
	setvbuf(logfile, NULL, _IOLBF, 0);
	setvbuf(logfile_debug, NULL, _IOLBF, 0);
}

void log_destory() {
	fclose(logfile);
	fclose(logfile_debug);
}

void log_msg_param(const char * format, ...) {
	int len;
	char buf[LOG_BUF_SIZE];
	va_list args;
	va_start(args, format);	
	len = vsnprintf(buf, LOG_BUF_SIZE - 1, format, args);
	va_end(args);
	fwrite(buf, 1, len, logfile);
}

void log_msg(const char * format, ...) {
	int len;
	char buf[LOG_BUF_SIZE];
	va_list args;
	va_start(args, format);	
	len = vsnprintf(buf, LOG_BUF_SIZE - 1, format, args);
	va_end(args);
	fwrite(buf, 1, len, logfile_debug);
}

void log_error(char * error_msg) {
	log_msg("ERROR! %s, %s\n", error_msg, strerror(errno));
}

/* After each request, it should append the following line to the log:
 * <time> <duration> <tput> <avg-tput> <bitrate> <server-ip> <chunkname>
 */
void log_parameters(unsigned long time, float duration, unsigned now_put,
	unsigned ave_put, unsigned bitrate, const char * server_ip, 
	const char * chunkname)
{
	log_msg_param("%lu %4.2f %u %u %u %s %s\n", time, duration, now_put, ave_put, 
		bitrate, server_ip, chunkname);
}
