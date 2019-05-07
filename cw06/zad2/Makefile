CFLAGS = -O1 -Wall
CC = gcc -Wall

all: server.c client.c chat.h
	make server
	make client
	make clear

main: main.c
	gcc -c $(CFLAGS) main.c -g -o main.o 
	gcc main.o -g $(CFLAGS) -L ./ -o main -lrt

server: server.c
	$(CC) server.c -o server -lrt
	
client: client.c
	$(CC) client.c -o client -lrt

test:
	./client client1 &
	./client client1 &
	./client client1 &
		

clear:
	rm  -f *.o *.a *.so