#ifndef __LOG__
#define __LOG__

#include <errno.h>
#include <stdio.h>

FILE* log_file_fd;

int open_log_file(char* filename);
int close_log_file(char* filename);
void mylog(char* format, ...);

#endif