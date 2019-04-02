CFLAGS = -O1 -Wall

all: monitor_buff.c monitor.c
	make monitor_buff
	make monitor	
	make clear

monitor_buff: monitor_buff.c
	gcc -c $(CFLAGS) monitor_buff.c -g -o monitor_buff.o
	gcc monitor_buff.o -g $(CFLAGS) -L ./ -o monitor_buff

monitor: monitor.c
	gcc -c $(CFLAGS) monitor.c -g -o monitor.o
	gcc monitor.o -g $(CFLAGS) -L ./ -o monitor

tester: tester.c
	gcc -c $(CFLAGS) tester.c -g -o tester.o
	gcc tester.o -g $(CFLAGS) -L ./ -o tester
	make clear

clear:
	rm  -f *.o *.a *.so