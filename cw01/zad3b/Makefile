CC := gcc
CFLAGS := -Wall -std=c99

all: clean static_main shared_main dl_main

run_all: run_static_main run_shared_main run_dl_main

all_optimized: clean clean_mains
	make OFLAGS=-O static_main
	make OFLAGS=-Os static_main
	make OFLAGS=-O0 static_main
	make OFLAGS=-O1 static_main
	make OFLAGS=-O2 static_main
	make OFLAGS=-O3 static_main
	make OFLAGS=-O shared_main
	make OFLAGS=-Os shared_main
	make OFLAGS=-O0 shared_main
	make OFLAGS=-O1 shared_main
	make OFLAGS=-O2 shared_main
	make OFLAGS=-O3 shared_main
	make OFLAGS=-O dl_main
	make OFLAGS=-Os dl_main
	make OFLAGS=-O0 dl_main
	make OFLAGS=-O1 dl_main
	make OFLAGS=-O2 dl_main
	make OFLAGS=-O3 dl_main

run_all_optimized: clean clean_mains
	make OFLAGS=-O static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-Os static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O0 static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O1 static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O2 static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O3 static_main
	./static_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-Os shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O0 shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O1 shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O2 shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O3 shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-Os dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O0 dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O1 dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O2 dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic
	make OFLAGS=-O3 dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 100000 dynamic

static_main: ../zad2/main.c archive
	$(CC) $(CFLAGS) $(OFLAGS) -c -DOUTPUT_FILE_NAME=\"raport3b.txt\" -DNO_OP_ITERS=1 -DSTATIC_LIB -I../zad1 ../zad2/main.c
	$(CC) $(CFLAGS) $(OFLAGS) -static -o static_main main.o -L. -lmbarr

shared_main: ../zad2/main.c shared
	$(CC) $(CFLAGS) $(OFLAGS) -c -DOUTPUT_FILE_NAME=\"raport3b.txt\" -DNO_OP_ITERS=1 -DSHARED_LIB -I../zad1 ../zad2/main.c
	$(CC) $(CFLAGS) $(OFLAGS) -o shared_main main.o -L. -lmbarr_shared -Wl,-rpath,.

dl_main: ../zad2/main.c shared
	$(CC) $(CFLAGS) $(OFLAGS) -c -DOUTPUT_FILE_NAME=\"raport3b.txt\" -DNO_OP_ITERS=1 -DDYNAMIC_RUNTIME_LOAD -I../zad1 ../zad2/main.c
	$(CC) $(CFLAGS) $(OFLAGS) -o dl_main main.o -L. -ldl -Wl,-rpath,.

static_main_O:
	make OFLAGS=-O static_main

static_main_Os:
	make OFLAGS=-Os static_main

static_main_O0:
	make OFLAGS=-O0 static_main

static_main_O1:
	make OFLAGS=-O1 static_main

static_main_O2:
	make OFLAGS=-O2 static_main

static_main_O3:
	make OFLAGS=-O3 static_main

shared_main_O:
	make OFLAGS=-O shared_main

shared_main_Os:
	make OFLAGS=-Os shared_main

shared_main_O0:
	make OFLAGS=-O0 shared_main

shared_main_O1:
	make OFLAGS=-O1 shared_main

shared_main_O2:
	make OFLAGS=-O2 shared_main

shared_main_O3:
	make OFLAGS=-O3 shared_main

dl_main_O:
	make OFLAGS=-O dl_main

dl_main_Os:
	make OFLAGS=-Os dl_main

dl_main_O0:
	make OFLAGS=-O0 dl_main

dl_main_O1:
	make OFLAGS=-O1 dl_main

dl_main_O2:
	make OFLAGS=-O2 dl_main

dl_main_O3:
	make OFLAGS=-O3 dl_main

run_static_main: clean static_main
	./static_main -c -f 20 -a 500 -s 800 1000 1000 dynamic

run_shared_main: clean shared_main
	./shared_main -c -f 20 -a 500 -s 800 1000 1000 dynamic

run_dl_main: clean dl_main
	./dl_main -c -f 20 -a 500 -s 800 1000 1000 dynamic

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