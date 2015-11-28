#ifndef _CONN_HANDLER_H_
#define _CONN_HANDLER_H_

#include <stdbool.h>

int conn_init(const char* fake_ip, const char * www_ip);
/* setup outbound connection to web server */
int proxy_conn_setup(char * server_ip);


#endif