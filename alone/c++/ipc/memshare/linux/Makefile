all: test_a test_b test_c

test_a: test_a.o memshare.o
	gcc -o test_a test_a.o memshare.o -pthread

test_b: test_b.o memshare.o
	gcc -o test_b test_b.o memshare.o -pthread
	
test_c: test_c.o memshare.o
	gcc -o test_c test_c.o memshare.o -pthread

memshare.o: memshare.c memshare.h
	gcc -c memshare.c

test_a.o: test_a.c memshare.h
	gcc -c test_a.c

test_b.o: test_b.c memshare.h
	gcc -c test_b.c

test_c.o: test_c.c memshare.h
	gcc -c test_c.c
