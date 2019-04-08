CFLAGS = -O1 -Wall

all: main.c
	make main
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o
	gcc main.o -g $(CFLAGS) -L ./ -o main

clear:
	rm  -f *.o *.a *.so