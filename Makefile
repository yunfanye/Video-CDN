CFLAGS = -Wall -g -I/usr/include/libxml2/ -I./ 
LIBS	= -L/usr/lib/x86_64-linux-gnu -lxml2
LDFLAGS = -lxml2
CC = gcc
objects = log.o mydns.o

nameserver_objects = log.o load_balancing.o nameserver.o util.o
dns_objects = util.o mydns.o
proxy_objects = y.tab.o lex.yy.o util.o name_util.o f4m_parser.o mydns.o time_util.o proxy_log.o rate_adapter.o conn_handler.o HTTP_handler.o proxy.o


proxy: $(proxy_objects)
		$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

mydns.o: mydns.c mydns.h util.h
		# $(CC) -o $@ $^ $(LDFLAGS)
		$(CC) $(FLAGS) $(LDFLAGS) -c $^

mydns: $(dns_objects)
		$(CC) -o $@ $^ $(LDFLAGS)

log: log.o
		$(CC) -o $@ $^ $(LDFLAGS)
		
lex.yy.c: lexer.l
	flex $^

y.tab.c: parser.y
	yacc -d $^

nameserver.o: nameserver.c nameserver.h load_balancing.h util.h mydns.h
		# $(CC) -o $@ $^ $(LDFLAGS)
		$(CC) $(FLAGS) $(LDFLAGS) -c $^

nameserver: $(nameserver_objects)
		$(CC) -o $@ $^ $(LDFLAGS)

f4m_parser: f4m_parser.o
	$(CC) -o $@ $^ $(LDFLAGS)

name_util: name_util.o
	$(CC) -o $@ $^ $(LDFLAGS)

load_balancing: load_balancing.o
		$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ *.gch lex.yy.c y.tab.c y.tab.h proxy_log_file.log proxy nameserver


# /proxy <log> <alpha> <listen-port> <fake-ip> <dns-ip> <dns-port>
# [<www-ip>]
	
run:
	./proxy proxy_log_file.log 0.5 1234 1.0.0.1 5.0.0.1 5050 4.0.0.1
	
debug: proxy
	(scp proxy proj3@128.237.165.185:project3/proxy;)
