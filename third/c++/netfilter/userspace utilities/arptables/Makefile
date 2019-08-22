ARPTABLES_VERSION:=0.0.4

KERNEL_DIR:=./
# default paths
PREFIX:=/usr/local
LIBDIR:=$(PREFIX)/lib
BINDIR:=$(PREFIX)/sbin
MANDIR:=$(PREFIX)/man
man8dir=$(MANDIR)/man8
SYSCONFIGDIR:=/etc/sysconfig
DESTDIR:=

MANS = arptables-legacy.8 arptables-save.8 arptables-restore.8

COPT_FLAGS:=-O2
CFLAGS:=$(COPT_FLAGS) -Wall -Wunused -I$(KERNEL_DIR)/include/ -Iinclude/ -DARPTABLES_VERSION=\"$(ARPTABLES_VERSION)\" #-g -DDEBUG #-pg # -DARPTC_DEBUG

ifndef ARPT_LIBDIR
ARPT_LIBDIR:=$(LIBDIR)/arptables
endif

include extensions/Makefile

all: arptables-legacy libarptc/libarptc.a

arptables.o: arptables.c
	$(CC) $(CFLAGS) -c -o $@ $<

arptables-standalone.o: arptables-standalone.c
	$(CC) $(CFLAGS) -c -o $@ $<

libarptc/libarptc.o: libarptc/libarptc.c libarptc/libarptc_incl.c
	$(CC) $(CFLAGS) -c -o $@ $<

libarptc/libarptc.a: libarptc/libarptc.o
	$(AR) rcs $@ $<

arptables-legacy: arptables-standalone.o arptables.o libarptc/libarptc.o $(EXT_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(DESTDIR)$(BINDIR)/arptables-legacy: arptables-legacy
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 0755 $< $@

tmp1:=$(shell printf $(BINDIR) | sed 's/\//\\\//g')
tmp2:=$(shell printf $(SYSCONFIGDIR) | sed 's/\//\\\//g')
.PHONY: scripts
scripts: arptables-save arptables-restore
	cat arptables-save | sed 's/__EXEC_PATH__/$(tmp1)/g' > arptables-save_
	install -m 0755 arptables-save_ $(DESTDIR)$(BINDIR)/arptables-save
	cat arptables-restore | sed 's/__EXEC_PATH__/$(tmp1)/g' > arptables-restore_
	install -m 0755 arptables-restore_ $(DESTDIR)$(BINDIR)/arptables-restore
	rm -f arptables-save_ arptables-restore_

.PHONY: install-man
install-man: $(MANS)
	[ -d "$(DESTDIR)$(man8dir)" ] || mkdir -p "$(DESTDIR)$(man8dir)"
	install -m 0644 $^ $(DESTDIR)$(man8dir)/

.PHONY: install
install: install-man $(DESTDIR)$(BINDIR)/arptables-legacy scripts

.PHONY: clean
clean:
	rm -f arptables-legacy
	rm -f *.o *~
	rm -f extensions/*.o extensions/*~
	rm -f libarptc/*.o libarptc/*~ libarptc/*.a
	rm -f include/*~ include/libarptc/*~

DIR:=arptables-v$(ARPTABLES_VERSION)
# This is used to make a new userspace release
.PHONY: release
release:
	make clean
	cd ..;find $(DIR) -exec touch {} \;;find $(DIR) -exec chmod o-r,g-r,o-w,g-w,o-x,g-x {} \;;tar -pc $(DIR) | gzip >$(DIR).tar.gz

