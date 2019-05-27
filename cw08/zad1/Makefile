all: main.c
	make main

main: main.c
	gcc main.c -o main -lpthread -lm

filter_gen: filter_gen.c
	gcc filter_gen.c -o filter_gen

simple_f: simple_filter.c
	gcc simple_filter.c -o simple_filter

test:
	echo "Image size: 640x480" > Times.txt
	echo "Type: BLOCK" >> Times.txt
	echo "Filter size: 10" >> Times.txt
	./filter_gen 10
	./main 1 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt 
	./main 2 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	echo "Filter size: 20" >> Times.txt
	./filter_gen 20
	./main 1 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	echo "Filter size: 40" >> Times.txt
	./filter_gen 40
	./main 1 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	echo "Filter size: 80" >> Times.txt
	./filter_gen 80
	./main 1 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 block venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	echo "Type: INTERLEAVE" >> Times.txt
	echo "Filter size: 10" >> Times.txt
	./filter_gen 10
	./main 1 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./filter_gen 20
	echo "Filter size: 20" >> Times.txt
	./main 1 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./filter_gen 40
	echo "Filter size: 40" >> Times.txt
	./main 1 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./filter_gen 80
	echo "Filter size: 80" >> Times.txt
	./main 1 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 2 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 4 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt
	./main 8 interleave venus2.ascii.pgm filter res2.ascii.pgm >> Times.txt


clear:
	rm -f *.o