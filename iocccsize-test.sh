#!/usr/bin/env bash
#
# iocccsize-test.sh - IOCCC Size Tool Tests
#
# "You are not expected to understand this" :-)
#
# Public Domain 1992, 2022 by Anthony Howe.  All rights released.
# With IOCCC minor mods in 2019, 2022 by chongo (Landon Curt Noll) ^oo^

export PATH='/bin:/usr/bin:/usr/local/bin:/usr/pkg/bin'
export ENV=''
export CDPATH=''
export LANG=C

EXIT_CODE=0
TESTDIR="./test-iocccsize"

usage()
{
	echo 'usage: iocccsize-test.sh [-bv][-t tool]'
	exit 2
}

__tool="./iocccsize"
__verbose=false

while getopts 'bvt:' opt; do
	case "$opt" in
	(b)
		__build="distclean"
		;;
	(v)
		__verbose=true
		__tool_args="-v1"
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

eval make "${__build}" all

if [ ! -d "${TESTDIR}" ]; then
	mkdir -p "${TESTDIR}"
fi

# Change DIGRAPHS and TRIGRAPHS according to limit_ioccc.h
if [ -f limit_ioccc.sh ]; then
	source limit_ioccc.sh
else
	export DIGRAPHS=true		# Assume #define DIGRAPHS.
	export TRIGRAPHS=true		# Assume #define TRIGRAPHS.
fi

get_wc()
{
	typeset file="$1"
	typeset field="$2"
	wc "$file" 2>/dev/null | sed -e's/^ *//; s/  */ /g' | cut -d' ' -f"$field"
}

test_size()
{
	typeset file="${TESTDIR}/$1"
	typeset expect="$2"
	typeset gross_count
	typeset got

	got=$("$__tool" $__tool_args "$file" 2>/dev/null)
	if $__verbose ; then
		gross_count=$(echo "$got" | cut -d' ' -f2)
		bytes=$(get_wc "$file" 3)
		if [ $gross_count != $bytes ]; then
			echo "FAIL $file: got $gross_count != wc $bytes"
			EXIT_CODE=1
			return
		fi
	else
		got=$(echo "$got" | cut -d' ' -f1)
		expect=$(echo "$expect" | cut -d' ' -f1)
	fi
	if [ "$expect" = "$got" ]; then
		echo "-OK- text_size $file: $got"
	else
		echo "FAIL text_size $file: got $got != expect $expect"
		EXIT_CODE=1
	fi
}

test_exit()
{
	typeset title="$1"
	typeset expect="$2"
	typeset got="$3"

	if [ $expect -eq $got ]; then
		echo "-OK- text_exit $title: $got"
	else
		echo "FAIL text_exit $title: got $got != expect $expect"
		EXIT_CODE=1
	fi
}

#######################################################################

"$__tool" -? 2>/dev/null
test_exit "usage" 2 $?

#######################################################################

"$__tool" -h 2>/dev/null
test_exit "help" 2 $?

#######################################################################

"$__tool" -V >/dev/null 2>/dev/null
test_exit "version" 3 $?

#######################################################################

"$__tool" -6 2>/dev/null
test_exit "prisoner" 6 $?

#######################################################################

"$__tool" -vWOOT 2>/dev/null
test_exit "bad_arg" 4 $?

#######################################################################

"$__tool" /tmp/WOOT 2>/dev/null
test_exit "bad_file" 6 $?

#######################################################################

printf 'int x;\r\n' >"${TESTDIR}/crlf.c"
test_size crlf.c "2 8 1"

#######################################################################

printf 'char str[] = "טיר";\r\n' >"${TESTDIR}/utf8.c"
test_size utf8.c "12 21 1"

#######################################################################

cat <<EOF >"${TESTDIR}/splitline0.c"
#define FOO \\
    int a = 666;
FOO;
EOF
test_size splitline0.c "19 36 1"

#######################################################################

cat <<EOF >"${TESTDIR}/comment0.c"
// comment one line "with a comment string" inside
int x;
EOF
test_size comment0.c "44 58 1"

#######################################################################

cat <<EOF >"${TESTDIR}/comment1.c"
/* comment block same line 'with a comment string' */
int x;
EOF
test_size comment1.c "46 61 1"

#######################################################################

cat <<EOF >"${TESTDIR}/comment2.c"
/*
comment block
multiline
*/
int x;
EOF
test_size comment2.c "27 37 1"

#######################################################################

cat <<EOF >"${TESTDIR}/comment3.c"
a//foo
EOF
test_size comment3.c "6 7 0"

#######################################################################

cat <<EOF >"${TESTDIR}/comment4.c"
/*/ int if for /*/
EOF
test_size comment4.c "14 19 0"

#######################################################################

cat <<EOF >"${TESTDIR}/comment5.c"
'"' "/*" foobar "*/"
EOF
test_size comment5.c "17 21 0"

#######################################################################

cat <<EOF >"${TESTDIR}/comment6.c"
char str[] = "string /* with */ comment";
EOF
test_size comment6.c "30 42 1"

#######################################################################

cat <<EOF >"${TESTDIR}/comment7.c"
// comment with backslash newline \\
int a = 666;
EOF
test_size comment7.c "37 49 0"

#######################################################################

cat <<EOF >"${TESTDIR}/quote0.c"
char str[] = "and\"or";
EOF
test_size quote0.c "16 24 1"

#######################################################################

cat <<EOF >"${TESTDIR}/quote1.c"
char squote = '\'';
EOF
test_size quote1.c "12 20 1"

#######################################################################

cat <<EOF >"${TESTDIR}/quote2.c"
char str[] = "'xor'";
EOF
test_size quote2.c "14 22 1"

#######################################################################

cat <<EOF >"${TESTDIR}/digraph.c"
char str<::> = "'xor'";
EOF
if $DIGRAPHS; then
	test_size digraph.c "14 24 1"
else
	test_size digraph.c "16 24 1"
fi

#######################################################################

cat <<EOF >"${TESTDIR}/trigraph0.c"
char str??(??) = "'xor'";
EOF
if $TRIGRAPHS; then
	test_size trigraph0.c "14 26 1"
else
	test_size trigraph0.c "18 26 1"
fi

#######################################################################

# Example from https://en.wikipedia.org/wiki/Digraphs_and_trigraphs#C
cat <<EOF >"${TESTDIR}/trigraph1.c"
// Will the next line be executed????????????????/
int a = 666;
EOF
if $TRIGRAPHS; then
	test_size trigraph1.c "49 64 0"
else
	test_size trigraph1.c "50 64 1"
fi

#######################################################################

# Example from https://en.wikipedia.org/wiki/Digraphs_and_trigraphs#C
cat <<EOF >"${TESTDIR}/trigraph2.c"
/??/
* A comment *??/
/
EOF
if $TRIGRAPHS; then
	test_size trigraph2.c "12 24 0"
else
	test_size trigraph2.c "18 24 0"
fi

#######################################################################

cat <<EOF >"${TESTDIR}/trigraph3.c"
#define FOO ??/
    int a = 666;
FOO;
EOF
if $TRIGRAPHS; then
	test_size trigraph3.c "19 38 1"
else
	test_size trigraph3.c "22 38 1"
fi

#######################################################################

cat <<EOF >"${TESTDIR}/main0.c"
int
main(int argc, char **argv)
{
	return 0;
}
EOF
test_size main0.c "22 47 4"

#######################################################################

cat <<EOF >"${TESTDIR}/hello.c"
#include <stdio.h>

int
main(int argc, char **argv)
{
	(void) printf("Hello world!\n");
	return 0;
}
EOF
test_size hello.c "58 101 6"

#######################################################################

# Digraph for #include and curlys.  Digraphs are tokens and are not
# translated like trigraphs.
cat <<EOF >"${TESTDIR}/hello_digraph.c"
%:    include <stdio.h>

int
main(int argc, char **argv)
<%
	(void) printf("Hello world!\n");
	return 0;
%>
EOF
if $DIGRAPHS; then
	test_size hello_digraph.c "60 108 6"
else
	test_size hello_digraph.c "70 108 5"	# bug? %:include a keyword?
fi

#######################################################################

# Trigraph for #include and curlys.  Trigraphs are translated, unlike
# digraphs which are tokens.
cat <<EOF >"${TESTDIR}/hello_trigraph.c"
??=    include <stdio.h>

int
main(int argc, char **argv)
??<
	(void) printf("Hello world!\n");
	return 0;
??>
EOF
if $TRIGRAPHS; then
	test_size hello_trigraph.c "58 111 6"
else
	test_size hello_trigraph.c "73 111 5"	# bug? ??=include a keyword?
fi

#######################################################################

cat <<EOF >"${TESTDIR}/include0.c"
#  include <stdio.h>
EOF
test_size include0.c "10 21 1"

#######################################################################

cat <<EOF >"${TESTDIR}/include1.c"
#  include <stdio.h>
#/*hi*/include <ctype.h>
EOF
test_size include1.c "26 46 2"

#######################################################################

cat <<EOF >"${TESTDIR}/curly0.c"
char str = "{ curly } ";
EOF
test_size curly0.c "12 25 1"

#######################################################################

# No spaces after curly braces in array initialiser.
cat <<EOF >"${TESTDIR}/curly1.c"
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
test_size curly1.c "119 192 6"

#######################################################################

# Spaces after curly braces in array initialiser.
cat <<EOF >"${TESTDIR}/curly2.c"
#include <stdlib.h>

#define STRLEN(s)		(sizeof (s)-1)

typedef struct {
	size_t length;
	const char *word;
} Word;

Word list[] = {
	{ STRLEN("spaced  curly"), "spaced  curly"} ,
	{ 0, NULL}
} ;
EOF
test_size curly2.c "113 196 6"

#######################################################################

cat <<EOF >"${TESTDIR}/semicolon0.c"
char str = "; xor; ";
EOF
test_size semicolon0.c "10 22 1"

#######################################################################

# Spaces after semicolons in for( ; ; ).
cat <<EOF >"${TESTDIR}/semicolon1.c"
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
test_size semicolon1.c "65 133 8"

#######################################################################

# No spaces after semicolons in for(;;).
cat <<EOF >"${TESTDIR}/semicolon2.c"
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
test_size semicolon2.c "67 127 8"

#######################################################################
# END
#######################################################################

exit ${EXIT_CODE}
