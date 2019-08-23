CC = gcc
CFLAGS = -I. # -g

libipq.a: libipq.o
	ld -i $< -o $@

libipq.o: libipq.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f libipq.o libipq.a
