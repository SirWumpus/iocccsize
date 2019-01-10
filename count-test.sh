#!/bin/ksh

export PATH='/bin:/usr/bin:/usr/local/bin:/usr/pkg/bin'
export ENV=''
export CDPATH=''
export LANG=C

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

cat <<-"EOF" >./decom
	#!/bin/sh
	if [ $# != 1 ]; then
		echo 'usage: decom file'
		exit 2
	fi
	gcc -fpreprocessed -dD -E -P "$1"
EOF
chmod a+x ./decom

if [ ! -d test ]; then
	mkdir test
fi

get_wc()
{
	typeset file="$1"
	typeset field="$2"
	typeset filter="$3"
	${filter:-./decom} $file | wc | sed -e's/^ *//; s/  */ /g' | cut -d' ' -f$field
}

test_size()
{
	typeset file="test/$1"
	typeset expect="$2"
	typeset filter="$3"
	typeset gross_count
	typeset keywords
	typeset got

	got=$(${filter:-./decom} $file | $__tool $__tool_args 2>&1 >/dev/null)
	if $__verbose ; then
		gross_count=$(echo $got | cut -d' ' -f2)
		bytes=$(get_wc $file 3 $filter)
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
		echo "FAIL $file: got $got != expect $expect"
	fi
}

cat <<EOF >test/comment0.c
// comment one line "with a comment string" inside
int x;
EOF
test_size comment0.c "2 7 1"
test_size comment0.c "44 58 1" cat

cat <<EOF >test/comment1.c
/* comment block same line */
int x;
EOF
test_size comment1.c "2 7 1"

cat <<EOF >test/comment2.c
/*
comment block multiline
*/
int x;
EOF
test_size comment2.c "2 7 1"

cat <<EOF >test/comment3.c
a//foo
EOF
test_size comment3.c "1 2 0"

cat <<EOF >test/comment4.c
/*/ int if for /*/
EOF
test_size comment4.c "0 0 0"

cat <<EOF >test/comment5.c
'"' "/*" foobar "*/"
EOF
test_size comment5.c "17 21 0"

cat <<EOF >test/comment6.c
char str[] = "string /* with */ comment";
EOF
test_size comment6.c "30 42 1"

cat <<EOF >test/quote0.c
char str[] = "and\"or";
EOF
test_size quote0.c "16 24 1"

cat <<EOF >test/quote1.c
char squote = '\'';
EOF
test_size quote1.c "12 20 1"

cat <<EOF >test/quote2.c
char str[] = "'xor'";
EOF
test_size quote2.c "14 22 1"

# 2019-01-10 Currently no special size exception for digraphs.
cat <<EOF >test/digraph.c
char str<::> = "'xor'";
EOF
test_size digraph.c "16 24 1"

# 2019-01-10 Currently no special size exception for trigraphs.
cat <<EOF >test/trigraph.c
char str??(??) = "'xor'";
EOF
test_size trigraph.c "18 26 1"

cat <<EOF >test/main0.c
int
main(int argc, char **argv)
{
	return 0;
}
EOF
test_size main0.c "22 47 4"

cat <<EOF >test/hello.c
#include <stdio.h>

int
main(int argc, char **argv)
{
	(void) printf("Hello world!\n");
	return 0;
}
EOF
test_size hello.c "58 100 6"

cat <<EOF >test/include0.c
# include <stdio.h>
EOF
test_size include0.c "10 20 1"

cat <<EOF >test/include1.c
# include <stdio.h>
#/*hi*/include <stdio.h>
EOF
test_size include1.c "20 40 2"

cat <<EOF >test/curly0.c
char str = "{ curly } ";
EOF
test_size curly0.c "12 25 1"

cat <<EOF >test/curly1.c
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
test_size curly1.c "119 188 6"

cat <<EOF >test/curly2.c
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
test_size curly2.c "114 191 6"

cat <<EOF >test/semicolon0.c
char str = "; xor; ";
EOF
test_size semicolon0.c "10 22 1"

cat <<EOF >test/semicolon1.c
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
test_size semicolon1.c "65 132 8"

cat <<EOF >test/semicolon2.c
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
test_size semicolon2.c "67 126 8"
