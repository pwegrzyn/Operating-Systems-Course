CC := gcc
CFLAGS := -Wall -std=c99

all: clean archive shared

archive: mbarr.c mbarr.h
	$(CC) $(CFLAGS) -c mbarr.c -o mbarr.o
	ar cr libmbarr.a mbarr.o

shared: mbarr.c mbarr.h
	$(CC) $(CFLAGS) -c -fPIC mbarr.c
	$(CC) $(CFLAGS) -shared -fPIC -o libmbarr_shared.so mbarr.o

clean: 
	rm -f *.o *.a *.so