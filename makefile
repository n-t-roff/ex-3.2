VERSION=3.2
#
# Ex skeletal makefile for version 7
#
# NB: This makefile doesn't indicate any dependencies on header files.
#
# Ex is very large - it may not fit on PDP-11's depending on the operating
# system and the cflags you turn on. Things that can be turned off to save
# space include LISPCODE (-l flag, showmatch and lisp options), UCVISUAL
# (visual \ nonsense on upper case only terminals), CHDIR (the undocumented
# chdir command.)
#
# Don't define VFORK unless your system has the VFORK system call,
# which is like fork but the two processes have only one data space until the
# child execs. This speeds up ex by saving the memory copy.
# -DVMUNIX makes an ex which can edit very large files (eg the w2a dictionary)
# this allows 200000 lines and about 16M byte temp files.
#
# If your system expands tabs to 4 spaces you should -DTABS=4 below
#
# Ex is likely to overflow the symbol table in your C compiler.
# It can use -t0 which is (purportedly) a C compiler with a larger
# symbol table.  The -t1 flag to the C compiler is for a C compiler
# which puts switch code in I space, increasing the text space size
# to the benefit of per-user data space.  If you don't have this it
# doesn't matter much. Another method, which works on v7 pdp-11's,
# is to use pcc for ex_io.c instead of cc.
#
PREFIX=	${DESTDIR}/usr/local
BINDIR=	${PREFIX}/bin
LIBDIR=	${PREFIX}/libexec
#
# Either none or both of the next two lines needs to be uncommented
#
D_SBRK=	-DUNIX_SBRK
MALLOC_O=mapmalloc.o
CTAGS=	${BINDIR}/ctags
DEBUGFLAGS=	-g -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	-Wall -Wextra \
	-fsanitize=undefined \
	-fsanitize=integer
#	-fsanitize=address
NONDEBUGFLAGS=	
_CFLAGS=	-DTABS=8 -DLISPCODE -DCHDIR -DUCVISUAL -DMACROS -DVMUNIX -DCBREAK \
	${D_SBRK} -DLIBDIR='"${LIBDIR}"' \
	${NONDEBUGFLAGS}
_LDFLAGS=-s
TERMLIB=	-ltinfo
OBJS=	ex.o ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_data.o ex_get.o \
	ex_io.o ex_put.o ex_re.o ex_set.o ex_subr.o ex_temp.o ex_tty.o \
	ex_v.o ex_vadj.o ex_vget.o ex_vmain.o ex_voperate.o \
	ex_vops.o ex_vops2.o ex_vops3.o ex_vput.o ex_vwind.o \
	printf.o ${MALLOC_O}

all:	a.out exrecover expreserve #tags

tags:
	${CTAGS} -w *.h *.c

${OBJS}: ex_vars.h ex.h makefile ex_tune.h

#ex_vars.h:
#	csh makeoptions ${_CFLAGS}

a.out: ${OBJS}
	${CC} ${CFLAGS} ${_CFLAGS} ${LDFLAGS} ${_LDFLAGS} ${OBJS} ${TERMLIB}

exrecover: exrecover.o
	${CC} ${CFLAGS} ${_CFLAGS} ${LDFLAGS} ${_LDFLAGS} exrecover.o \
	    -o exrecover

expreserve: expreserve.o
	${CC} ${CFLAGS} ${_CFLAGS} ${LDFLAGS} ${_LDFLAGS} expreserve.o \
	    -o expreserve

.c.o:
	${CC} ${CFLAGS} ${_CFLAGS} -c $<

clean:
#	If we dont have ex we cant make it so dont rm ex_vars.h
	-rm -f a.out exrecover expreserve ex${VERSION}strings strings core trace tags
	-rm -f *.o x*.[cs]

distclean: clean
	rm -rf Makefile config.log

install: ${BINDIR} ${LIBDIR}
	install a.out ${BINDIR}/ex
	for i in vi view edit; do \
		ln -sf ${BINDIR}/ex ${BINDIR}/$$i; \
	done
	for i in recover preserve; do \
		install ex$$i ${LIBDIR}/ex${VERSION}$$i; \
	done

uninstall:
	for i in ex vi view edit; do \
		rm -f ${BINDIR}/$$i; \
	done
	for i in recover preserve; do \
		rm -f ${LIBDIR}/ex${VERSION}$$i; \
	done

${BINDIR} ${LIBDIR}:
	mkdir -p $@

lint:
	lint ex.c ex_?*.c
	lint -u exrecover.c
	lint expreserve.c
