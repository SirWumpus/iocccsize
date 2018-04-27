.POSIX :

O := .o
E :=

.SUFFIXES :
.SUFFIXES : .c .h .i $O $E

########################################################################
### No further configuration beyond this point.
########################################################################

top_srcdir	:= ..
PACKAGE 	:= None
TAR_I		:= -T
CFLAGS		:= -g -std=c90 -Wall -Wno-char-subscripts -pedantic
CFLAGS89	:= -g -std=c89 -Wall -Wno-char-subscripts -pedantic
LDFLAGS		:=
LIBS		:=
IT		:= count

.c.i:
	${CC} -E ${CFLAGS} $*.c >$*.i

#######################################################################

all: build

build: ${IT}

clean:
	-rm -f ${PACKAGE}.i ${PACKAGE}$O ${PACKAGE}$E *.stackdump *.core 2>/dev/null

distclean: clean
	-rm -fr ${IT}$E test 2>/dev/null

test: ./${IT}-test.sh -v
