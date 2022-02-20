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
WERROR		:=
#WERROR		:= -Werror
CFLAGS		:= -g -std=c11 -Wall -Wextra -Wno-char-subscripts -pedantic ${WERROR}
CFLAGS89	:= -g -std=c89 -Wall -Wextra -Wno-char-subscripts -pedantic ${WERROR}
CPPFLAGS	:=
LIBS		:=

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

iocccsize: iocccsize.c rule_count.c iocccsize.h iocccsize_err.h # XXX - gross hack - fix me - XXX
	${CC} ${CPPFLAGS} ${CFLAGS} iocccsize.c rule_count.c ${LIBS} -o iocccsize    # XXX - gross hack - fix me - XXX

clean:
	-rm -f ${PROJ}.i ${PROJ}$O *.stackdump *.core 2>/dev/null

distclean clobber: clean
	-rm -fr ${PROJ}$E test a.out VERSION 2>/dev/null
	-rm -fr ${PROJ}.dSYM 2>/dev/null
	-rm -fr test-${PROJ} 2>/dev/null

test: build
	./${PROJ}-test.sh -v

version:
	git describe --tags --abbrev=0 >VERSION

tar: version
	git archive --format=tar.gz --prefix=${PROJ}-${VERSION}/ ${VERSION} >../${PROJ}-${VERSION}.tar.gz

#${PROJ}$E: ${PROJ}.c	# XXX - gross hack - fix me - XXX
#	${CC} ${CFLAGS} ${CPPFLAGS} -DWITH_MAIN -o$*$E $*.c	# XXX - gross hack - fix me - XXX
