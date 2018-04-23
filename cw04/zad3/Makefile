CC := gcc
CFLAGS := -Wall -std=c99 -g

all: clean zad3

zad3: zad3.o
	$(CC) $(CFLAGS) -o zad3 zad3.o

zad3.o: zad3.c
	$(CC) $(CFLAGS) -c zad3.c

clean: 
	rm -f *.o zad3