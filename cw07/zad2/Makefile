CC := gcc
CFLAGS := -Wall -std=c99 -g
LDLIBS := -lrt -lpthread

all: clean barber client

barber: barber.o
	$(CC) $(CFLAGS) -o barber barber.o $(LDLIBS)

barber.o: barber.c
	$(CC) $(CFLAGS) -c barber.c

client: client.o
	$(CC) $(CFLAGS) -o client client.o $(LDLIBS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean: 
	rm -f *.o barber client