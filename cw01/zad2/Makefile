CC := gcc
CFLAGS := -Wall -std=c99

all: clean main

main: main.c archive
	$(CC) $(CFLAGS) -c -DOUTPUT_FILE_NAME=\"raport2.txt\" -DNO_OP_ITERS=1 -I../zad1 main.c
	$(CC) $(CFLAGS) -static -o main main.o -L. -lmbarr

run_main_example: all
	./main -c -f 20 -a 500 -s 800 1000 100000 dynamic

archive: ../zad1/mbarr.c ../zad1/mbarr.h
	$(CC) $(CFLAGS) -c ../zad1/mbarr.c -o mbarr.o
	ar cr libmbarr.a mbarr.o

clean: 
	rm -f *.o *.a *.so main