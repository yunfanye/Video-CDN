CFLAGS = -Wall -g
LDFLAGS = 
CC = gcc
objects = log.o mydns.o
nameserver_objects = log.o load_balancing.o nameserver.o util.o
dns_objects = util.o mydns.o
proxy_objects = time_util.o proxy_log.o rate_adapter.o conn_handler.o proxy.o

proxy: $(proxy_objects)
		$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

mydns.o: mydns.c mydns.h util.h
		# $(CC) -o $@ $^ $(LDFLAGS)
		$(CC) $(FLAGS) $(LDFLAGS) -c $^

mydns: $(dns_objects)
		$(CC) -o $@ $^ $(LDFLAGS)

log: log.o
		$(CC) -o $@ $^ $(LDFLAGS)

nameserver.o: nameserver.c nameserver.h load_balancing.h util.h mydns.h
		# $(CC) -o $@ $^ $(LDFLAGS)
		$(CC) $(FLAGS) $(LDFLAGS) -c $^

nameserver: $(nameserver_objects)
		$(CC) -o $@ $^ $(LDFLAGS)

load_balancing: load_balancing.o
		$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ *.gch
