CC := gcc
CFLAGS := -Wall -std=c99 -g

all: clean program

program: main.o
	$(CC) $(CFLAGS) -o program main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean: 
	rm -f *.o program