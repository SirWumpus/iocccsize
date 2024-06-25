.POSIX :

O := .o
E :=

.SUFFIXES :
.SUFFIXES : .c .h .i $O $E

########################################################################
### No further configuration beyond this point.
########################################################################

top_srcdir	:= ..
PROJ 		:= iocccsize
TAR_I		:= -T
DBG		:= -DNDEBUG
CFLAGS		:= -std=c11 -Wall -Wextra -Wno-char-subscripts -pedantic ${WERROR} ${DBG}
CPPFLAGS	:=
LIBS		:=
EPREFIX		:= ${HOME}
BINDIR		:= ${EPREFIX}/bin

VERSION		:= $$(git describe --tags)

.c.i:
	${CC} -E ${CFLAGS} ${CPPFLAGS} $*.c >$*.i

.c$O:
	${CC} ${CFLAGS} ${CPPFLAGS} -c $*.c

.c$E:
	${CC} ${CFLAGS} ${CPPFLAGS} -o$*$E $*.c

#######################################################################

all: build

build: ${PROJ}

clean:
	-rm -f ${PROJ}.i ${PROJ}$O rule_count$O *.stackdump *.core 2>/dev/null

distclean clobber: clean
	-rm -fr ${PROJ}$E ${PROJ}.dSYM test-${PROJ} a.out VERSION 2>/dev/null

test: build
	./${PROJ}-test.sh -v

install: build
	install -d ${BINDIR}
	install -p ${PROJ}$E ${BINDIR}

version:
	git describe --tags >VERSION

tar: version
	git archive --format=tar.gz --prefix=${PROJ}-${VERSION}/ ${VERSION} >../${PROJ}-${VERSION}.tar.gz

iocccsize$E: iocccsize.h iocccsize.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DVERSION='"'${VERSION}'"' -DWITH_MAIN -oiocccsize$E iocccsize.c ${LIBS}

rule_count$O: iocccsize.h iocccsize.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c -orule_count$O iocccsize.c
