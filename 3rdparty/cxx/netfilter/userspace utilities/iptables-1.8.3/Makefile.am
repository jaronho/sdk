# -*- Makefile -*-

ACLOCAL_AMFLAGS  = -I m4
AUTOMAKE_OPTIONS = foreign subdir-objects

SUBDIRS          = libiptc libxtables
if ENABLE_DEVEL
SUBDIRS         += include
endif
if ENABLE_LIBIPQ
SUBDIRS         += libipq
endif
SUBDIRS         += utils
# Depends on libxtables:
SUBDIRS         += extensions
# Depends on extensions/libext.a:
SUBDIRS         += iptables

if ENABLE_NFTABLES
confdir		= $(sysconfdir)
dist_conf_DATA	= etc/ethertypes
endif

.PHONY: tarball
tarball:
	rm -Rf /tmp/${PACKAGE_TARNAME}-${PACKAGE_VERSION};
	pushd ${top_srcdir} && git archive --prefix=${PACKAGE_TARNAME}-${PACKAGE_VERSION}/ HEAD | tar -C /tmp -x && popd;
	pushd /tmp/${PACKAGE_TARNAME}-${PACKAGE_VERSION} && ./autogen.sh && popd;
	tar -C /tmp -cjf ${PACKAGE_TARNAME}-${PACKAGE_VERSION}.tar.bz2 --owner=root --group=root ${PACKAGE_TARNAME}-${PACKAGE_VERSION}/;
	rm -Rf /tmp/${PACKAGE_TARNAME}-${PACKAGE_VERSION};

config.status: extensions/GNUmakefile.in \
	include/xtables-version.h.in include/iptables/internal.h.in
