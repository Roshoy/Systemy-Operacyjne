CFLAGS = -O1 -Wall

all: main.c
	make main
	make main_sh
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o
	gcc main.o -g $(CFLAGS) -L ./ -o main

main_sh: main_sh.c
	gcc -c $(CFLAGS) main_sh.c -g -o main_sh.o
	gcc main_sh.o -g $(CFLAGS) -L ./ -o main_sh
	


clear:
	rm  -f *.o *.a *.so