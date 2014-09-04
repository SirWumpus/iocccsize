#
# Copyright 2012, 2013 by Anthony Howe.
#

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
IT		:= ioccc

.c.i:
	${CC} -E ${CFLAGS} $*.c >$*.i

#######################################################################

all: build

build: ${IT}

clean:
	-rm -f ${PACKAGE}.i ${PACKAGE}$O ${PACKAGE}$E *.stackdump *.core 2>/dev/null

distclean: clean
	-rm -f ${IT}$E ../${PACKAGE}.c ../${PACKAGE}$E 2>/dev/null

${PACKAGE}$O: ${PACKAGE}.c
	${CC} ${CFLAGS} -c ${PACKAGE}.c

${PACKAGE}$E: ${PACKAGE}.c
	${CC} -DTEST ${CFLAGS} ${LDFLAGS} -o${PACKAGE}$E ${PACKAGE}.c ${LIBS}

${IT}$E: ${IT}.c
	${CC} ${CFLAGS} -o${IT}$E ${IT}.c

#
# Exercise the IOCCC size tool.
#
test: test-k test-wc test-rcount

#
# -k should pass through the source with block comments intact.
#
test-k: ${IT}$E
	@{ t="-k keep comments" ;\
	if ./${IT} -k <${IT}.c 2>/dev/null | diff - ${IT}.c ; then echo "-OK- $$t" ; else echo "FAIL $$t" ; fi ;\
	}

#
# First 3 counters should match wc(1)
#
test-wc: ${IT}$E
	@{ t="wc counts" ;\
	a=$$(./${IT} -s -k <${IT}.c 2>&1 | cut -d' ' -f1-3) ;\
	b=$$(wc ${IT}.c | sed -e's/^ *//; s/  */ /g' | cut -d' ' -f1-3) ;\
	if [ "$$a" = "$$b" ]; then echo "-OK- $$t"; else echo "FAIL $$t: expected '$$b', got '$$a'"; fi ;\
	}

#
# Show counters for different option combinations.
#
test-show: ${IT}$E
	./${IT} -s <${IT}.c
	./${IT} -s -r <${IT}.c
	./${IT} -s -k <${IT}.c
	./${IT} -s -r -k <${IT}.c

#
# Reserved word counter should be same for all option combinations.
#
test-rcount: ${IT}$E
	@{ t="rcount" ;\
	./${IT} -s <${IT}.c 2>&1 | cut -d" " -f 6 >/tmp/$$$$.tmp ;\
	./${IT} -s -r <${IT}.c 2>&1 | cut -d" " -f 6 >>/tmp/$$$$.tmp ;\
	./${IT} -s -k <${IT}.c 2>&1 | cut -d" " -f 6 >>/tmp/$$$$.tmp ;\
	./${IT} -s -r -k <${IT}.c 2>&1 | cut -d" " -f 6 >>/tmp/$$$$.tmp ;\
	n=$$(sort -u /tmp/$$$$.tmp | wc -l | sed -e's/^ *//') ;\
	if [ $$n -eq 1 ]; then echo "-OK- $$t"; else echo "FAIL $$t: expected 1 unique line, got $$n"; echo "see make test-show"; fi ;\
	rm -f /tmp/$$$$.tmp ;\
	}

${top_srcdir}/${PACKAGE}.c size: ${PACKAGE}.c transform.sed ${IT}$E
	@echo '***************************************************************'
	@sed -f transform.sed ${PACKAGE}.c | sed -e '/^[ 	]*$$/d' | ./${IT}$E -r >${top_srcdir}/${PACKAGE}.c
	@echo '***************************************************************'

${top_srcdir}/${PACKAGE}$E: ${top_srcdir}/${PACKAGE}.c
	cd ${top_srcdir}; ${CC} ${CFLAGS89} ${LDFLAGS} -o${PACKAGE}$E ${PACKAGE}.c ${LIBS}; cd -

entry : ${top_srcdir}/${PACKAGE}$E

tar: ${top_srcdir}/${PACKAGE}.c MANIFEST.TXT
	tar -C ../${top_srcdir} ${TAR_I} MANIFEST.TXT -zcf 2012.tar.gz
