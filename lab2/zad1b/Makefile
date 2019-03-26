CFLAGS = -O1 -Wall

all: 
	tests
	make clear

tests:
	rm -f wyniki.txt 
	
	../zad1a/main test 9000 1
	../zad1a/main test 13500 1
	../zad1a/main test 9000 4
	../zad1a/main test 13500 4
	../zad1a/main test 9000 512
	../zad1a/main test 13500 512
	../zad1a/main test 9000 1024
	../zad1a/main test 13500 1024
	../zad1a/main test 9000 4096
	../zad1a/main test 13500 4096
	../zad1a/main test 9000 8192
	../zad1a/main test 13500 8192
	mv ../zad1a/wyniki.txt .
clear:
	rm  -f ../zad1a/data*