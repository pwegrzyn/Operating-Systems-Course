CC := gcc
CFLAGS := -Wall -std=c99

all: clean program

run_tests: clean program
	./program generate dane5000_4.txt 5000 4
	./program generate dane2500_4.txt 2500 4
	./program generate dane5000_512.txt 5000 512
	./program generate dane2500_512.txt 2500 512
	./program generate dane5000_4096.txt 5000 4096
	./program generate dane2500_4096.txt 2500 4096
	./program generate dane5000_8192.txt 5000 8192
	./program generate dane2500_8192.txt 2500 8192
	./program copy dane5000_4.txt dane5000_4_kopia.txt 5000 1024 lib
	./program copy dane5000_4.txt dane5000_4_kopia2.txt 5000 1024 sys
	./program copy dane2500_4.txt dane2500_4_kopia.txt 2500 512 sys
	./program copy dane2500_4.txt dane2500_4_kopia2.txt 2500 512 lib
	./program copy dane5000_512.txt dane5000_512_kopia.txt 5000 1024 sys
	./program copy dane5000_512.txt dane5000_512_kopia2.txt 5000 1024 lib
	./program copy dane2500_512.txt dane2500_512_kopia.txt 2500 512 lib
	./program copy dane2500_512.txt dane2500_512_kopia2.txt 2500 512 sys
	./program copy dane5000_4096.txt dane5000_4096_kopia.txt 5000 1024 lib
	./program copy dane5000_4096.txt dane5000_4096_kopia2.txt 5000 1024 sys
	./program copy dane2500_4096.txt dane2500_4096_kopia.txt 2500 512 sys
	./program copy dane2500_4096.txt dane2500_4096_kopia2.txt 2500 512 lib
	./program copy dane5000_8192.txt dane5000_8192_kopia.txt 5000 1024 sys
	./program copy dane5000_8192.txt dane5000_8192_kopia2.txt 5000 1024 lib
	./program copy dane2500_8192.txt dane2500_8192_kopia.txt 2500 512 lib
	./program copy dane2500_8192.txt dane2500_8192_kopia2.txt 2500 512 sys
	./program sort dane5000_4_kopia.txt 5000 4 lib
	./program sort dane5000_4_kopia2.txt 5000 4 sys
	./program sort dane2500_4_kopia.txt 2500 4 lib
	./program sort dane2500_4_kopia2.txt 2500 4 sys
	./program sort dane5000_512_kopia.txt 5000 512 lib
	./program sort dane5000_512_kopia2.txt 5000 512 sys
	./program sort dane2500_512_kopia.txt 2500 512 lib
	./program sort dane2500_512_kopia2.txt 2500 512 sys
	./program sort dane5000_4096_kopia.txt 5000 4096 lib
	./program sort dane5000_4096_kopia2.txt 5000 4096 sys
	./program sort dane2500_4096_kopia.txt 2500 4096 lib
	./program sort dane2500_4096_kopia2.txt 2500 4096 sys
	./program sort dane5000_8192_kopia.txt 5000 8192 lib
	./program sort dane5000_8192_kopia2.txt 5000 8192 sys
	./program sort dane2500_8192_kopia.txt 2500 8192 lib
	./program sort dane2500_8192_kopia2.txt 2500 8192 sys

program: main.o iofuncs.o
	$(CC) $(CFLAGS) -o program main.o iofuncs.o

iofuncs.o: iofuncs.c iofuncs.h
	$(CC) $(CFLAGS) -c iofuncs.c

main.o: main.c iofuncs.h
	$(CC) $(CFLAGS) -c -DTIME_MEASURE_FILE=\"wyniki.txt\" main.c

clean: 
	rm -f *.o program *.txt