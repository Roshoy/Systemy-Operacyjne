CFLAGS = -O1 -Wall

all: catcher.c sender.c
	make catcher
	make sender
	make clear

catcher: catcher.c
	gcc -c $(CFLAGS) catcher.c -g -o catcher.o
	gcc catcher.o -g $(CFLAGS) -L ./ -o catcher

sender: sender.c
	gcc -c $(CFLAGS) sender.c -g -o sender.o
	gcc sender.o -g $(CFLAGS) -L ./ -o sender

clear:
	rm  -f *.o *.a *.so