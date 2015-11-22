#include <stdio.h>
#include <sys/time.h>
#include "time_util.h"

/* return timestamp in milliseconds */
unsigned long milli_time() {
	struct timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000 * 1000 + time.tv_usec;
}
