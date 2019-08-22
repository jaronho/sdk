AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = $(regular_CFLAGS)

pkglib_LTLIBRARIES = ulogd_output_IPFIX.la

ulogd_output_IPFIX_la_SOURCES = ulogd_output_IPFIX.c ipfix.c
ulogd_output_IPFIX_la_LDFLAGS = -avoid-version -module
