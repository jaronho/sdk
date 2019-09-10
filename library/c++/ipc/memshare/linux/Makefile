all: test_a test_b test_c

test_a: test_a.o memshare.o queue.o
	gcc -o test_a test_a.o memshare.o queue.o -pthread

test_b: test_b.o memshare.o queue.o
	gcc -o test_b test_b.o memshare.o queue.o -pthread
	
test_c: test_c.o memshare.o queue.o
	gcc -o test_c test_c.o memshare.o queue.o -pthread

queue.o: queue.c queue.h
	gcc -c queue.c

memshare.o: memshare.c memshare.h queue.h
	gcc -c memshare.c

test_a.o: test_a.c memshare.h
	gcc -c test_a.c

test_b.o: test_b.c memshare.h
	gcc -c test_b.c

test_c.o: test_c.c memshare.h
	gcc -c test_c.c