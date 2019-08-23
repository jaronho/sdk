CFLAGS = -O2 -fomit-frame-pointer -Wall -DDEBUG -g 

CC = gcc
LD = ld
LIBIPQ = libipq/libipq.a

OBJS = client.o ctl.o ctlfd.o ipq.o queue.o ctlmsg.o ipqmpd.o pktid.o 
OBJINC = -I./libipq

all:	ipqmpd libipqmpd/libipqmpd.a nfqtest/nfqtest

ipqmpd: $(OBJS) $(LIBIPQ)
	$(CC) -o $@ $(OBJS) $(LIBIPQ)

.c.o: 
	$(CC) $(CFLAGS) $(OBJINC) -o $@ -c $<

nfqtest/nfqtest: nfqtest/nfqtest.c 
	$(CC) $(CFLAGS) -I. -o $@ -i $< libipqmpd/libipqmpd.a

libipqmpd/libipqmpd.o: libipqmpd/libipqmpd.c 
	$(CC) $(CFLAGS) -I. -o $@ -c $<

libipqmpd/libipqmpd.a: libipqmpd/libipqmpd.o ctlmsg.o
	$(LD) -o $@ -i $< ctlmsg.o

clean: 
	rm -f core ipqmpd nfqtest/nfqtest $(OBJS) libipqmpd/libipqmpd.a  libipqmpd/libipqmpd.o
