include $(top_srcdir)/Make_global.am

ACLOCAL_AMFLAGS  = -I m4

man_MANS = nfacct.8
EXTRA_DIST = $(man_MANS) Make_global.am m4

SUBDIRS = include src
DIST_SUBDIRS = include src

LIBS = @LIBMNL@ @LIBNETFILTER_ACCT@
