CFLAGS = -O2 -fomit-frame-pointer -I..
CC = gcc

nfqtest: nfqtest.o ../libipqmpd/libipqmpd.a
	$(CC) -o nfqtest nfqtest.o ../libipqmpd/libipqmpd.a

nfqtest.o: nfqtest.c Makefile
	$(CC) $(CFLAGS) -o nfqtest.o -c nfqtest.c
