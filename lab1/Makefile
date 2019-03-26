CFLAGS=-Os

all:
	make static
	make shared
	make main_static
	make main_shared
	make main_dynamic

static:
	make clear
	gcc -c -g $(CFLAGS) -Wall libfind.c
	ar rcsv libfind.a libfind.o

shared:
	make clear
	gcc -c $(CFLAGS) -fPIC libfind.c
	gcc -shared $(CFLAGS) -Wl,-soname,libfind.so.1 -o libfind.so.1.0.1 libfind.o -lc
	rm libfind.so.1
	ln -s libfind.so.1.0.1 libfind.so.1
	ln -s libfind.so.1 libfind.so

test_dynamic: dyn_test.c
	make shared
	gcc dyn_test.c -ldl -o $(CFLAGS) dyn_test -ld -rpath=.


test_shared: test.c
	make shared
	gcc test.c -lfind $(CFLAGS) -L ./ -o shared_test

test: test.c
	#rm -f test.o test
	gcc -c -g -Wall test.c $(CFLAGS) -o test.o 
	gcc test.o -g -l find -L ./ $(CFLAGS) -o test

main_static: main.c
	gcc -c $(CFLAGS) -g -Wall main.c -o main.o
	gcc main.o -g $(CFLAGS) -l find -L ./ -o main -Wl,-rpath,.

main_shared:
	gcc main.c -lfind $(CFLAGS) -L ./ -o main_sh -Wl,-rpath,.

main_dynamic:
	gcc main.c -ldl $(CFLAGS) -o main_dyn -L ./ -D DLL

clear:
	rm -f *.o *.a *.so