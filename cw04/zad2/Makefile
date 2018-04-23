CC := gcc
CFLAGS := -Wall -std=c99 -g

all: clean zad2

zad2: zad2.o
	$(CC) $(CFLAGS) -o zad2 zad2.o

zad2.o: zad2.c
	$(CC) $(CFLAGS) -c zad2.c

clean: 
	rm -f *.o zad2