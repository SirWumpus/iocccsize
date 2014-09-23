/*
 * ioccc.c
 *
 * IOCCC Secondary Source Size Rule
 *
 * Public Domain 1992, 2014 by Anthony Howe.  All rights released.
 *
 *
 * SYNOPSIS
 *
 * 	ioccc [-krs] < input
 *
 *	-k	keep comment blocks
 *	-r	count C reserved words as 1 byte
 *	-s	suppress source output, write only the IOCCC 2nd size count
 *
 * DESCRIPION
 *
 *	Taking input from standard input, apply the IOCCC Source Size Rule
 *	for 2001.  The program's offical length is written to standard error.
 *	Also filter out C comment blocks (-k to keep) sending the modified
 *	source to standard output.
 *
 *	The 2001 Source Size Rule was:
 *
 *	Your entry must be <= 4096 bytes in length.  The number of octets
 *	excluding whitespace (tab, space, newline, formfeed, return), and
 *	excluding any ';', '{' or '}' followed by whitespace or end of file,
 *	must be <= 2048.
 *
 *	2013 Source Size Rule is (-r):
 *
 *	Your entry must be <= 4096 bytes in length.  The number of octets
 *	excluding whitespace (tab, space, newline, formfeed, return), and
 *	excluding any ';', '{' or '}' followed by whitespace or end of file,
 *	and where C reserved words, including a subset of preprocessor words,
 *	count as 1 byte, must be <= 2053 (first prime after 2048).
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#define FLAG_SILENCE		1
#define FLAG_KEEP		2
#define FLAG_RESERVED		4

#define BUFFER_SIZE		512

char usage[] =
"usage:  ioccc [-krs] < prog.c\n"
"\n"
"-k\t\tkeep block comments\n"
"-r\t\tcount C reserved words as 1 byte\n"
"-s\t\tsuppress source output, write only the offical size\n"
;

#define STRLEN(s)		(sizeof (s)-1)

typedef struct {
	size_t length;
	const char *word;
} Word;

/*
 * C reserved words, plus a few #preprocessor tokens, that count as 1
 *
 * NOTE: For a good list of reserved words in C, see:
 *
 *	http://www.bezem.de/pdf/ReservedWordsInC.pdf
 *
 * by Johan Bezem of JB Enterprises:
 *
 *	See http://www.bezem.de/en/
 */
static Word cwords[] = {
	/* Yes Virginia, we left #define off the list on purpose! */
	{ STRLEN("#elif"), "#elif" },
	{ STRLEN("#else"), "#else" },
	{ STRLEN("#endif"), "#endif" },
	{ STRLEN("#error"), "#error" },
	{ STRLEN("#ident"), "#ident" },
	{ STRLEN("#if"), "#if" },
	{ STRLEN("#ifdef"), "#ifdef" },
	{ STRLEN("#include"), "#include" },
	{ STRLEN("#line"), "#line" },
	{ STRLEN("#pragma"), "#pragma" },
	{ STRLEN("#sccs"), "#sccs" },
	{ STRLEN("#warning"), "#warning" },
	/**/
	{ STRLEN("_Alignas"), "_Alignas" },
	{ STRLEN("_Alignof"), "_Alignof" },
	{ STRLEN("_Atomic"), "_Atomic" },
	{ STRLEN("_Bool"), "_Bool" },
	{ STRLEN("_Complex"), "_Complex" },
	{ STRLEN("_Generic"), "_Generic" },
	{ STRLEN("_Imaginary"), "_Imaginary" },
	{ STRLEN("_Noreturn"), "_Noreturn" },
	{ STRLEN("_Pragma"), "_Pragma" },
	{ STRLEN("_Static_assert"), "_Static_assert" },
	{ STRLEN("_Thread_local"), "_Thread_local" },
	/**/
	{ STRLEN("alignas"), "alignas" },
	{ STRLEN("alignof"), "alignof" },
	{ STRLEN("and"), "and" },
	{ STRLEN("and_eq"), "and_eq" },
	{ STRLEN("auto"), "auto" },
	{ STRLEN("bitand"), "bitand" },
	{ STRLEN("bitor"), "bitor" },
	{ STRLEN("bool"), "bool" },
	{ STRLEN("break"), "break" },
	{ STRLEN("case"), "case" },
	{ STRLEN("char"), "char" },
	{ STRLEN("compl"), "compl" },
	{ STRLEN("const"), "const" },
	{ STRLEN("continue"), "continue" },
	{ STRLEN("default"), "default" },
	{ STRLEN("do"), "do" },
	{ STRLEN("double"), "double" },
	{ STRLEN("else"), "else" },
	{ STRLEN("enum"), "enum" },
	{ STRLEN("extern"), "extern" },
	{ STRLEN("false"), "false" },
	{ STRLEN("float"), "float" },
	{ STRLEN("for"), "for" },
	{ STRLEN("goto"), "goto" },
	{ STRLEN("I"), "I" },
	{ STRLEN("if"), "if" },
	{ STRLEN("inline"), "inline" },
	{ STRLEN("int"), "int" },
	{ STRLEN("long"), "long" },
	{ STRLEN("noreturn"), "noreturn" },
	{ STRLEN("not"), "not" },
	{ STRLEN("not_eq"), "not_eq" },
	{ STRLEN("or"), "or" },
	{ STRLEN("or_eq"), "or_eq" },
	{ STRLEN("register"), "register" },
	{ STRLEN("restrict"), "restrict" },
	{ STRLEN("return"), "return" },
	{ STRLEN("short"), "short" },
	{ STRLEN("signed"), "signed" },
	{ STRLEN("sizeof"), "sizeof" },
	{ STRLEN("static"), "static" },
	{ STRLEN("static_assert"), "static_assert" },
	{ STRLEN("struct"), "struct" },
	{ STRLEN("switch"), "switch" },
	{ STRLEN("thread_local"), "thread_local" },
	{ STRLEN("true"), "true" },
	{ STRLEN("typedef"), "typedef" },
	{ STRLEN("union"), "union" },
	{ STRLEN("unsigned"), "unsigned" },
	{ STRLEN("void"), "void" },
	{ STRLEN("volatile"), "volatile" },
	{ STRLEN("while"), "while" },
	{ STRLEN("xor"), "xor" },
	{ STRLEN("xor_eq"), "xor_eq" },
	/**/
	{ 0, NULL }
};

Word *
find_member(Word *table, const char *string)
{
	Word *w;
	for (w = table; w->length != 0; w++) {
		if (strncmp(string, w->word, w->length) == 0
		&& !isalnum(string[w->length]) && string[w->length] != '_')
			return w;
	}
	return NULL;
}

size_t
read_line(char *buf, size_t size)
{
	int ch;
	size_t length;

	if (buf == NULL || size == 0)
		return 0;

	for (size--, length = 0; length < size; ) {
		if ((ch = fgetc(stdin)) == EOF)
			break;
		if (ch == '\0')
			ch = ' ';
		buf[length++] = ch;
		if (ch == '\n')
			break;
	}

	buf[length] = '\0';

	return length;
}

/*
 * Count octets and strip comments.  The stripped C input is sent to
 * standard output.  If -s is set, then suppress the source output.
 * The various counters are sent to standard error.
 *
 * The counter output format is:
 *
 *	lcount wcount bcount icount isaved rcount rsaved
 *
 * where
 *	lcount	line count (same as wc)
 *	wcount	word count (same as wc)
 *	bcount	byte count (same as wc)
 *	icount	IOCCC secondary size rule count
 *	isaved	bytes saved by secondary size rule (bcount - icount)
 *	rcount	number of C reserved words
 *	rsaved	number of octets saved with -r
 */
int
count(int flags)
{
	Word *w;
	int span;
	char *p, buf[BUFFER_SIZE];
	int lcount, wcount, bcount;
	int is_comment, is_word, dquote, escape;
	int count, keywords, saved, kw_saved;

	/* Start of buffer sentinel. */
	buf[0] = ' ';

	count = saved = 0;
	keywords = kw_saved = 0;
	lcount = wcount = bcount = 0;
	is_comment = is_word = dquote = escape = 0;

	while (0 < read_line(buf+1, sizeof (buf)-1)) {
		if (!(flags & FLAG_KEEP)) {
			/* Leading whitespace before comment block? */
			span = strspn(buf+1, "\t ");

			/* Split / * across reads? */
			if (buf[1+span] == '/' && buf[2+span] == '\0') {
				(void) ungetc('/', stdin);
				continue;
			}

			if (buf[1+span] == '/' && buf[2+span] == '/') 
				continue;
			if (buf[1+span] == '/' && buf[2+span] == '*') {
				/* Strip leading whitespace before comment block. */
				is_comment = 1;
			}
		}

		for (p = buf+1; *p != '\0'; p++) {
			/* Within quoted string? */
			if (dquote) {
				/* Escape _this_ character. */
				if (escape)
					escape = 0;

				/* Escape next character. */
				else if (*p == '\\')
					escape = 1;

				/* Close quoted string? */
				else if (*p == '"')
					dquote = 0;
			}

			/* Not quote string. */
			else {
				/* In C comment block? */
				if (is_comment) {
					/* Split * / across reads? */
					if (*p == '*' && p[1] == '\0') {
						ungetc('*', stdin);
						break;
					}

					/* End of comment block? */
					if (*p == '*' && p[1] == '/') {
						is_comment = 0;

						if (!(flags & FLAG_KEEP)) {
							/* Strip whitespace and newline
							 * trailing closing comment.
							 */
							p += 1 + strspn(p+2, " \t\r\n");
						}
					}

					if (!(flags & FLAG_KEEP)) {
						/* Strip octets in comment block. */
						continue;
					}
				}

				/* Split / / or / * across reads? */
				else if (*p == '/' && p[1] == '\0') {
					ungetc('/', stdin);
					break;
				}

				/* Start of comment line? */
				else if (*p == '/' && p[1] == '/') {
					if (!(flags & FLAG_KEEP)) {
						/* Strip comment to end of buffer. */
						break;
					}
				}

				/* Start of comment block? */
				else if (*p == '/' && p[1] == '*') {
					/* Begin comment block. */
					is_comment = 1;

					if (!(flags & FLAG_KEEP)) {
						/* Strip comment block. */
						p++;
						continue;
					}
				}

				/* C reserved word? */
				else if (!isalnum(p[-1]) && p[-1] != '_'
				&& (w = find_member(cwords, p)) != NULL) {
					keywords++;
					if (flags & FLAG_RESERVED) {
						bcount += w->length;
						if (!is_word) {
							is_word = 1;
							wcount++;
						}

						if (!(flags & FLAG_SILENCE))
							fputs(w->word, stdout);

						/* Count reserved word as one. */
						kw_saved += w->length - 1;
						p += w->length - 1;
						count++;
						continue;
					}
				}

				/* Open quoted string? */
				else if (*p == '"') {
					dquote = 1;
				}
			}

			if (!(flags & FLAG_SILENCE))
				fputc(*p, stdout);

			bcount++;
			if (*p == '\n')
				lcount++;

			/* Ignore all whitespace. */
			if (isspace(*p)) {
				is_word = 0;
				saved++;
				continue;
			} else if (!is_word) {
				is_word = 1;
				wcount++;
			}

			/* Ignore curly braces and semicolons when followed
			 * by any whitspace or EOF.
			 */
			if (strchr("{;}", *p) != NULL
			&& (isspace(p[1]) || p[1] == '\0')) {
				saved++;
				continue;
			}

			/* Count this octet. */
			count++;
		}
	}

	/* The Ugly Truth */
	fprintf(
		stderr, "%d %d %d %d %d %d %d\n",
		lcount, wcount, bcount, count, saved,
		keywords, kw_saved
	);

	return count;
}

int
main(int argc, char **argv)
{
	int ch;
	int flags = 0;

	while ((ch = getopt(argc, argv, "krs")) != -1) {
		switch (ch) {
		case 'k':
			flags |= FLAG_KEEP;
			break;
		case 'r':
			flags |= FLAG_RESERVED;
			break;
		case 's':
			flags |= FLAG_SILENCE;
			break;
		default:
			fprintf(stderr, "%s\n", usage);
			return 2;
		}
	}

	(void) count(flags);

	return 0;
}
