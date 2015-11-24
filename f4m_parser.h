#ifndef _F4M_PARSER_H_
#define _F4M_PARSER_H_

#include <libxml/parser.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>
#include "util.h"

struct bitrate{
	int bitrate;
	struct bitrate* next;
};

unsigned * extract_bitrate_list(char * buf, int size);
// struct bitrate* extract_bitrate_list(char* buf, int size);

#endif
