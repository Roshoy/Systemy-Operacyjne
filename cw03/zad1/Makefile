CFLAGS = -O1 -Wall

all: main.c
	make main
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o
	gcc main.o -g $(CFLAGS) -L ./ -o main

test: test.c
	gcc -c $(CFLAGS) test.c -g -o test.o
	gcc test.o -g $(CFLAGS) -L ./ -o test
	make clear

clear:
	rm  -f *.o *.a *.so