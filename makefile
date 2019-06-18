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
CFLAGS		:= -g -std=c11 -Wall -Wno-char-subscripts -pedantic
CFLAGS89	:= -g -std=c89 -Wall -Wno-char-subscripts -pedantic
LDFLAGS		:=
LIBS		:=

.c.i:
	${CC} -E ${CFLAGS} $*.c >$*.i

.c$E:
	${CC} ${CFLAGS} -o$*$E $*.c

#######################################################################

all: build

build: ${PROJ}

clean:
	-rm -f ${PROJ}.i ${PROJ}$O *.stackdump *.core 2>/dev/null

distclean: clean
	-rm -fr ${PROJ}$E test decom a.out 2>/dev/null

test: build
	./${PROJ}-test.sh -v

