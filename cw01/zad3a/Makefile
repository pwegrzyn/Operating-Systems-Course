CC := gcc
CFLAGS := -Wall -std=c99

all: clean static_main shared_main dl_main

run_all: run_static_main run_shared_main run_dl_main

static_main: ../zad2/main.c archive
	$(CC) $(CFLAGS) -c -DOUTPUT_FILE_NAME=\"raport3a.txt\" -DNO_OP_ITERS=1 -DSTATIC_LIB -I../zad1 ../zad2/main.c
	$(CC) $(CFLAGS) -static -o static_main main.o -L. -lmbarr

shared_main: ../zad2/main.c shared
	$(CC) $(CFLAGS) -c -DOUTPUT_FILE_NAME=\"raport3a.txt\" -DNO_OP_ITERS=1 -DSHARED_LIB -I../zad1 ../zad2/main.c
	$(CC) $(CFLAGS) -o shared_main main.o -L. -lmbarr_shared -Wl,-rpath,.

dl_main: ../zad2/main.c shared
	$(CC) $(CFLAGS) -c -DOUTPUT_FILE_NAME=\"raport3a.txt\" -DNO_OP_ITERS=1 -DDYNAMIC_RUNTIME_LOAD -I../zad1 ../zad2/main.c
	$(CC) $(CFLAGS) -o dl_main main.o -L. -ldl -Wl,-rpath,.

run_static_main: clean static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic

run_shared_main: clean shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic

run_dl_main: clean dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic

archive: ../zad1/mbarr.c ../zad1/mbarr.h
	$(CC) $(CFLAGS) -c ../zad1/mbarr.c -o mbarr.o
	ar cr libmbarr.a mbarr.o

shared: ../zad1/mbarr.c ../zad1/mbarr.h
	$(CC) $(CFLAGS) -c -fPIC ../zad1/mbarr.c
	$(CC) $(CFLAGS) -shared -fPIC -o libmbarr_shared.so mbarr.o

clean: 
	rm -f *.o *.a *.so

clean_mains:
	rm -f *_main