#!/bin/ksh

usage()
{
	echo 'usage: count-test.sh [-bv][-t tool]'
	exit 2
}

__tool="./count"
__verbose=false

while getopts 'bvt:' opt; do
	case "$opt" in
	(b)
		__build="distclean"
		;;
	(v)
		__verbose=true
		__tool_args="-v"
		;;
	(t)
		__tool="$OPTARG"
		;;
	(*)
		usage
	esac
done
shift $(($OPTIND - 1))
#if [ $# -lt 1 ]; then
#        usage
#fi

make -f count.mk ${__build} all

if [ ! -f './decom' ]; then
	cat <<-EOF
		#!/bin/sh
		if [ $# != 1 ]; then
			echo 'usage: decom file'
			exit 2
		fi
		gcc -fpreprocessed -dD -E -P "$1"
	EOF
	chmod a+x ./decom
fi
if [ ! -d test ]; then
	mkdir test
fi

get_wc()
{
	./decom $1 | wc | sed -e's/^ *//; s/  */ /g' | cut -d' ' -f$2
}

test()
{
	typeset file="test/$1"; shift
	typeset expect="$1"; shift
	typeset gross_count
	typeset keywords
	typeset got

	got=$(./decom $file | $__tool $__tool_args 2>&1 >/dev/null)
	if $__verbose ; then
		gross_count=$(echo $got | cut -d' ' -f2)
		bytes=$(get_wc $file 3)
		if [ $gross_count != $bytes ]; then
			echo "FAIL $file: wc $bytes != $gross_count"
			return
		fi
	else
		got=$(echo $got | cut -d' ' -f1)
		expect=$(echo $expect | cut -d' ' -f1)
	fi
	if [ "$expect" = "$got" ]; then
		echo "-OK- $file: $got"
	else
		echo "FAIL $file: $got != $expect"
	fi
}

cat <<EOF >test/comment0.c
// comment one line
int x;
EOF
test comment0.c "2 7 1"

cat <<EOF >test/comment1.c
/* comment block same line */
int x;
EOF
test comment1.c "2 7 1"

cat <<EOF >test/comment2.c
/*
comment block same line
*/
int x;
EOF
test comment2.c "2 7 1"

cat <<EOF >test/comment3.c
a//foo
EOF
test comment3.c "1 2 0"

cat <<EOF >test/comment4.c
/*/ int if for /*/
EOF
test comment4.c "0 0 0"

cat <<EOF >test/comment5.c
'"' "/*" foobar "*/"
EOF
test comment5.c "17 21 0"

cat <<EOF >test/main0.c
int
main(int argc, char **argv)
{
	return 0;
}
EOF
test main0.c "22 47 4"

cat <<EOF >test/hello.c
#include <stdio.h>

int
main(int argc, char **argv)
{
	(void) printf("Hello world!\n");
	return 0;
}
EOF
test hello.c "58 100 6"

cat <<EOF >test/include0.c
# include <stdio.h>
EOF
test include0.c "10 20 1"

cat <<EOF >test/include1.c
# include <stdio.h>
#/*hi*/include <stdio.h>
EOF
test include1.c "20 40 2"

cat <<EOF >test/curly0.c
// No spaces after curly braces in array initialiser.
#include <stdlib.h>

#define STRLEN(s)		(sizeof (s)-1)

typedef struct {
	size_t length;
	const char *word;
} Word;

Word list[] = {
	{STRLEN("abutted curly"), "abutted curly"},
	{0, NULL}
};
EOF
test curly0.c "119 188 6"

cat <<EOF >test/curly1.c
// Spaces after curly braces in array initialiser.
#include <stdlib.h>

#define STRLEN(s)		(sizeof (s)-1)

typedef struct {
	size_t length;
	const char *word;
} Word;

Word list[] = {
	{ STRLEN("spaced  curly"), "spaced  curly"} ,
	{ 0, NULL}
};
EOF
test curly1.c "114 191 6"

cat <<EOF >test/semicolon0.c
// Spaces after semicolons in for( ; ; ).
#include <stdio.h>

int
main(int argc, char **argv)
{
	int i;
	for (i = 0; i < 3; i++) {
		(void) printf("%d\n", i);
	}
	return 0;
}
EOF
test semicolon0.c "65 132 8"

cat <<EOF >test/semicolon1.c
// No spaces after semicolons in for(;;).
#include <stdio.h>

int
main(int argc, char **argv)
{
	int i;
	for (i=0;i<3;i++) {
		(void) printf("%d\n", i);
	}
	return 0;
}
EOF
test semicolon1.c "67 126 8"
