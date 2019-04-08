CFLAGS = -O1 -Wall

all: master.c slave.c main.c
	make main
	make master
	make slave
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o
	gcc main.o -g $(CFLAGS) -L ./ -o main

slave: slave.c
	gcc -c $(CFLAGS) slave.c -g -o slave.o
	gcc slave.o -g $(CFLAGS) -L ./ -o slave

master: master.c
	gcc -c $(CFLAGS) master.c -g -o master.o
	gcc master.o -g $(CFLAGS) -L ./ -o master

clear:
	rm  -f *.o *.a *.so