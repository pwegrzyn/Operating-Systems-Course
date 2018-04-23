CC := gcc
CFLAGS := -Wall -std=c99 -g

all: clean exceed_limits

exceed_limits: exceed_limits.o
	$(CC) $(CFLAGS) -o exceed_limits exceed_limits.o

exceed_limits.o: exceed_limits.c
	$(CC) $(CFLAGS) -c exceed_limits.c

clean: 
	rm -f *.o exceed_limits