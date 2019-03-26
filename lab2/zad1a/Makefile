CFLAGS = -O1 -Wall

all: main.c
	make clear
	make main
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o
	gcc main.o -g $(CFLAGS) -L ./ -o main

tests:
	rm -f wyniki.txt
	./main test 9000 1
	./main test 13500 1
	./main test 9000 4
	./main test 13500 4
	./main test 9000 512
	./main test 13500 512
	./main test 9000 1024
	./main test 13500 1024
	./main test 9000 4096
	./main test 13500 4096
	./main test 9000 8192
	./main test 13500 8192
clear:
	rm  -f *.o *.a *.so