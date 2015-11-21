// This file contains utility functions for writing to the log file specified
// by the command line input for lisod server.
// Log information includes: 
// 1. IP addresses and browser information/settings as coming from connected clients.
// 2. All errors encountered to the log file.

// include library needed for vfprintf functions
#include <stdarg.h>
#include "log.h"


// file pointer set by server

// This function returns -1 if fails opening file
int open_log_file(char* filename){
	log_file_fd = fopen(filename, "w");
	if(log_file_fd==NULL){
		mylog("Error opening log file, please check input path of it.\n");
		return -1;
	}
	return 1;
}

int close_log_file(char* filename){
	if(log_file_fd!=NULL){
		fclose(log_file_fd);
	}
	return 1;
}

// This function writes a message with format and variable length arguments
// The usage is like printf(format, arguments).
void mylog(char* format, ...){
	va_list arguments;
	va_start(arguments, format);
	vfprintf(log_file_fd, format, arguments);
	va_end(arguments);
	fflush(log_file_fd);
}

void dns_log(int time, char* client_ip, char* query_name, char* response_ip){
	mylog("%d %s %s %s\n", time, client_ip, query_name, response_ip);
}