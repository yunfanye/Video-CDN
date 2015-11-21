CFLAGS = -Wall -g
LDFLAGS = 
CC = gcc
objects = log.o mydns.o

mydns: mydns.o
		$(CC) -o $@ $^ $(LDFLAGS)

log: log.o
		$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o