# $Id: Makefile.in,v 1.15 2007/09/19 11:49:44 toad32767 Exp $

CC = gcc
RANLIB = ranlib
AR = ar
CFLAGS = -DHAVE_CONFIG_H -I. -Ilibaiff -g -O2 -ansi -pedantic -Wall -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Wsign-compare -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wno-unreachable-code

# for GCC4. comment for GCC3
CFLAGS += -Wextra -Wmissing-field-initializers -Wno-discard-qual

prefix = /usr/local

objs = iff.o aifx.o lpcm.o g711.o \
       float32.o libaiff.o extended.o pascal.o

all: libaiff.a

install: libaiff.a
	cp libaiff.a $(prefix)/lib
	$(RANLIB) $(prefix)/lib/libaiff.a
	chmod 0444 $(prefix)/lib/libaiff.a
	cp -R ./libaiff $(prefix)/include/libaiff
	chmod 0755 $(prefix)/include/libaiff
	chmod 0444 $(prefix)/include/libaiff/*
	rm -f $(prefix)/include/libaiff/config.h.in

uninstall: deinstall

deinstall:
	rm -f $(prefix)/lib/libaiff.a
	rm -rf $(prefix)/include/libaiff

libaiff.a: $(objs)
	@echo "#	creating $@"
	$(AR) cru $@ $(objs)
	$(RANLIB) $@

clean:
	rm -f *.o libaiff.a

cleanconfig:
	rm -f config.log config.status libaiff/config.h Makefile

cleanall: clean cleanconfig

.c.o:
	@echo "#	compiling $@"
	$(CC) $(CFLAGS) -c $< -o $@


