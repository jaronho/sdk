include $(top_srcdir)/Make_global.am

sbin_PROGRAMS = nft-sync

AM_YFLAGS = -d

CLEANFILES = config-parser.c		\
	     config-scanner.c

nft_sync_SOURCES = event.c		\
		   logging.c		\
		   msg_buff.c		\
		   mnl.c		\
		   server.c		\
		   client.c		\
		   tcp.c		\
		   timer.c		\
		   main.c		\
		   utils.c		\
		   fd.c			\
		   config-parser.y	\
		   config-scanner.l
nft_sync_LDADD = ${LIBMNL_LIBS} ${LIBNFTNL_LIBS} -lev

# yacc and lex generate dirty code
config-scanner.o config-parser.o: AM_CFLAGS += -Wno-missing-prototypes -Wno-missing-declarations -Wno-implicit-function-declaration -Wno-nested-externs -Wno-undef -Wno-redundant-decls

EXTRA_DIST = config-parser.h
