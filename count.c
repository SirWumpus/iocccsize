/*
 * SYNOPSIS
 *
 * 	ioccc [-v] < input
 *
 * DESCRIPION
 *
 *	Reading a C source file from standard input, apply the IOCCC
 *	source size rules as explained in the Guidelines.  The source
 *	code is passed through on standard output.  The source's net
 *	length is written to standard error; with -v option the net
 *	length, gross length, and matched keyword count are written.
 *
 *	The entry's gross size in bytes must be less than equal to 4096
 *	bytes in length.
 *
 *	The entry's net size in bytes must be less than equal to 2053
 *	bytes (first prime after 2048).  The net size is computed as
 *	follows:
 *
 *	The size tool counts most C reserved words (keyword, secondary,
 *	and selected preprocessor keywords) as 1.  The size tool counts all
 *	other octets as 1 excluding ASCII whitespace, and excluding any
 *	';', '{' or '}' followed by ASCII whitespace, and excluding any
 *	';', '{' or '}' octet immediately before the end of file.
 */

#include <err.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>

#define MAX_SIZE		4096	/* IOCCC Rule 2a */
#define MAX_COUNT		2053	/* IOCCC Rule 2b */
#define STRLEN(s)		(sizeof (s)-1)

static char usage[] = "usage: ioccc [-v] < prog.c";

static int debug;

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
typedef struct {
	size_t length;
	const char *word;
} Word;

static Word cwords[] = {
	/* Yes Virginia, we left #define off the list on purpose! */
	{ STRLEN("#elif"), "#elif" } ,
	{ STRLEN("#else"), "#else" } ,
	{ STRLEN("#endif"), "#endif" } ,
	{ STRLEN("#error"), "#error" } ,
	{ STRLEN("#ident"), "#ident" } ,
	{ STRLEN("#if"), "#if" } ,
	{ STRLEN("#ifdef"), "#ifdef" } ,
	{ STRLEN("#ifndef"), "#ifndef" } ,
	{ STRLEN("#include"), "#include" } ,
	{ STRLEN("#line"), "#line" } ,
	{ STRLEN("#pragma"), "#pragma" } ,
	{ STRLEN("#sccs"), "#sccs" } ,
	{ STRLEN("#warning"), "#warning" } ,

	{ STRLEN("_Alignas"), "_Alignas" } ,
	{ STRLEN("_Alignof"), "_Alignof" } ,
	{ STRLEN("_Atomic"), "_Atomic" } ,
	{ STRLEN("_Bool"), "_Bool" } ,
	{ STRLEN("_Complex"), "_Complex" } ,
	{ STRLEN("_Generic"), "_Generic" } ,
	{ STRLEN("_Imaginary"), "_Imaginary" } ,
	{ STRLEN("_Noreturn"), "_Noreturn" } ,
	{ STRLEN("_Pragma"), "_Pragma" } ,
	{ STRLEN("_Static_assert"), "_Static_assert" } ,
	{ STRLEN("_Thread_local"), "_Thread_local" } ,

	{ STRLEN("alignas"), "alignas" } ,
	{ STRLEN("alignof"), "alignof" } ,
	{ STRLEN("and"), "and" } ,
	{ STRLEN("and_eq"), "and_eq" } ,
	{ STRLEN("auto"), "auto" } ,
	{ STRLEN("bitand"), "bitand" } ,
	{ STRLEN("bitor"), "bitor" } ,
	{ STRLEN("bool"), "bool" } ,
	{ STRLEN("break"), "break" } ,
	{ STRLEN("case"), "case" } ,
	{ STRLEN("char"), "char" } ,
	{ STRLEN("compl"), "compl" } ,
	{ STRLEN("const"), "const" } ,
	{ STRLEN("continue"), "continue" } ,
	{ STRLEN("default"), "default" } ,
	{ STRLEN("do"), "do" } ,
	{ STRLEN("double"), "double" } ,
	{ STRLEN("else"), "else" } ,
	{ STRLEN("enum"), "enum" } ,
	{ STRLEN("extern"), "extern" } ,
	{ STRLEN("false"), "false" } ,
	{ STRLEN("float"), "float" } ,
	{ STRLEN("for"), "for" } ,
	{ STRLEN("goto"), "goto" } ,
	{ STRLEN("if"), "if" } ,
	{ STRLEN("inline"), "inline" } ,
	{ STRLEN("int"), "int" } ,
	{ STRLEN("long"), "long" } ,
	{ STRLEN("noreturn"), "noreturn" } ,
	{ STRLEN("not"), "not" } ,
	{ STRLEN("not_eq"), "not_eq" } ,
	{ STRLEN("or"), "or" } ,
	{ STRLEN("or_eq"), "or_eq" } ,
	{ STRLEN("register"), "register" } ,
	{ STRLEN("restrict"), "restrict" } ,
	{ STRLEN("return"), "return" } ,
	{ STRLEN("short"), "short" } ,
	{ STRLEN("signed"), "signed" } ,
	{ STRLEN("sizeof"), "sizeof" } ,
	{ STRLEN("static"), "static" } ,
	{ STRLEN("static_assert"), "static_assert" } ,
	{ STRLEN("struct"), "struct" } ,
	{ STRLEN("switch"), "switch" } ,
	{ STRLEN("thread_local"), "thread_local" } ,
	{ STRLEN("true"), "true" } ,
	{ STRLEN("typedef"), "typedef" } ,
	{ STRLEN("union"), "union" } ,
	{ STRLEN("unsigned"), "unsigned" } ,
	{ STRLEN("void"), "void" } ,
	{ STRLEN("volatile"), "volatile" } ,
	{ STRLEN("while"), "while" } ,
	{ STRLEN("xor"), "xor" } ,
	{ STRLEN("xor_eq"), "xor_eq" } ,

	{ 0, NULL }
};

static Word *
find_member(Word *table, const char *string)
{
	Word *w;
	for (w = table; w->length != 0; w++) {
		if (strcmp(string, w->word) == 0) {
			return w;
		}
	}
	return NULL;
}

static int
read_ch(FILE *fp)
{
	int ch, next_ch;
	static char trigraph[] = "=(/)'<!>-";
	static char asciimap[] = "#[\\]^{|}~";

	while ((ch = fgetc(fp)) != EOF) {
		if (ch < '\0' || 128 <= ch) {
			errx(1, "NUL or non-ASCII characters");
		}
		if (ch == '\r') {
			/* Discard bare CR and those part of CRLF. */
			continue;
		}
		if ((next_ch = fgetc(fp)) == EOF) {
			break;
		}
		if (ch == '\\' && next_ch == '\n') {
			/* ISO C11 section 5.1.1.2 Translation Phases
			 * point 2 discards backslash newlines.
			 */
			continue;
		}
		if (ch == '?' && next_ch == '?') {
			/* ISO C11 section 5.2.1.1 Trigraph Sequences */
			char *tri;
			next_ch = fgetc(fp);
			if ((tri = strchr(trigraph, next_ch)) == NULL) {
				errx(1, "Invalid trigraph \?\?%c", (char) next_ch);
			}
			/* Mapped trigraphs count as 1 byte. */
			return asciimap[tri - trigraph];
		}
		(void) ungetc(next_ch, fp);
		break;
	}

	return ch;
}

static void
rule_count(FILE *fp)
{
	char word[32];
	size_t gross_count = 0, net_count = 0, keywords = 0;
	int ch, next_ch, quote = 0, escape = 0, is_comment = 0, wordi = 0;

	while ((ch = read_ch(fp)) != EOF) {
		/* Future gazing. */
		next_ch = read_ch(fp);
		(void) ungetc(next_ch, fp);

		/* Within quoted string? */
		if (quote != 0) {
			/* Escape _this_ character. */
			if (escape) {
				escape = 0;
			}

			/* Escape next character. */
			else if (ch == '\\') {
				escape = 1;
			}

			/* Close matching quote? */
			else if (ch == quote) {
				quote = 0;
			}
		}

		/* Within comment to end of line? */
		else if (is_comment == 1 && ch == '\n') {
			is_comment = 0;
		}

		/* Within comment block? */
		else if (is_comment == 2 && ch == '*' && next_ch == '/') {
			is_comment = 0;
		}

		/* Start of comment to end of line? */
		else if (ch == '/' && next_ch == '/') {
			is_comment = 1;
		}

		/* Start of comment block? */
		else if (ch == '/' && next_ch == '*') {
			is_comment = 2;
		}

		/* Open single or double quote? */
		else if (ch == '\'' || ch == '"') {
			quote = ch;
		}

		(void) fputc(ch, stdout);

		/* Sanity check against file size and wc(1) byte count. */
		gross_count++;

		/* End of possible keyword?  Care with #word as there can
		 * whitespace or comments between # and word.
		 */
		if ((word[0] != '#' || 1 < wordi) && !isalnum(ch) && ch != '_' && ch != '#') {
			if (find_member(cwords, word) != NULL) {

				/* Count keyword as 1. */
#define HUH
#ifdef HUH
/* Disabled and the counts match the old iocccsize tool.
 * Enabled the net_count is smaller, but the logic seems correct.
 */
				net_count = net_count - wordi + 1;
#endif
				keywords++;
			}
			word[wordi = 0] = '\0';
		}

		/* Ignore all whitespace. */
		if (isspace(ch)) {
			continue;
		}

		/* Ignore begin/end block and end of statement. */
		if (strchr("{;}", ch) != NULL && (isspace(next_ch) || next_ch == EOF)) {
			continue;
		}

		/* Collect next word not in a string or comment. */
		if (quote == is_comment && (isalnum(ch) || ch == '_' || ch == '#')) {
			if (sizeof (word) <= wordi) {
				wordi = 0;
			}
			word[wordi++] = (char) ch;
			word[wordi] = '\0';
		}

		net_count++;
	}

	(void) fprintf(stderr, debug ? "%lu %lu %lu\n" : "%lu\n", net_count, gross_count, keywords);
}

int
main(int argc, char **argv)
{
	int ch;

	while ((ch = getopt(argc, argv, "v")) != -1) {
		switch (ch) {
		case 'v':
			debug++;
			break;
		default:
			errx(2, "%s", usage);
		}
	}

	(void) setvbuf(stdin, NULL, _IOLBF, 0);

	/* The Count - 1 Muha .. 2 Muhaha .. 3 Muhahaha ... */
	rule_count(stdin);

	return 0;
}
