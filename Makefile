CFLAGS = -Wall -g
LDFLAGS = 
CC = gcc
objects = log.o mydns.o
proxy_objects = time_util.o proxy_log.o mydns.o rate_adapter.o conn_handler.o proxy.o

proxy: $(proxy_objects)
		$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

mydns: mydns.o
		$(CC) -o $@ $^ $(LDFLAGS)

log: log.o
		$(CC) -o $@ $^ $(LDFLAGS)

nameserver: nameserver.o
		$(CC) -o $@ $^ $(LDFLAGS)

load_balancing: load_balancing.o
		$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~
