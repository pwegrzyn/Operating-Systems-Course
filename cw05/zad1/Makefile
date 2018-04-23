CC := gcc
CFLAGS := -Wall -std=c99 -g

all: clean zad1

zad1: zad1.o
	$(CC) $(CFLAGS) -o zad1 zad1.o

zad1.o: zad1.c
	$(CC) $(CFLAGS) -c zad1.c

clean: 
	rm -f *.o zad1