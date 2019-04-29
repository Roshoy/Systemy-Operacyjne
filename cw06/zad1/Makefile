CFLAGS = -O1 -Wall
CC = gcc -Wall

all: server.c client.c
	make server
	make client
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o
	gcc main.o -g $(CFLAGS) -L ./ -o main

server: server.c
	$(CC) server.c -o server
	
client: client.c
	$(CC) client.c -o client


clear:
	rm  -f *.o *.a *.so