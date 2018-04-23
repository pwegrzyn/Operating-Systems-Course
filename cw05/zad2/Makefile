CC := gcc
CFLAGS := -Wall -std=c99 -g

all: clean master slave example

master: master.o
	$(CC) $(CFLAGS) -o master master.o

master.o: master.c
	$(CC) $(CFLAGS) -c master.c

slave: slave.o
	$(CC) $(CFLAGS) -o slave slave.o

slave.o: slave.c
	$(CC) $(CFLAGS) -c slave.c

example: example.o
	$(CC) $(CFLAGS) -o example example.o

example.o: example.c
	$(CC) $(CFLAGS) -c example.c

clean: 
	rm -f *.o master slave example