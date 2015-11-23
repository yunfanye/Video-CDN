#define _GNU_SOURCE

#include <string.h>
#include "f4m_parser.h"

/* extract bitrate value from
 * <media url="/myvideo/high" bitrate="1708" width="1920" height="1080"/> 
 * return malloc-ed bitrate list
 */
unsigned * extract_bitrate_list(char * buf, int size) {
	
	buf[size] = '\0';
	int index = 0;
	while(strcasestr(buf + index, "<meida") != NULL) {
	
	}
}
