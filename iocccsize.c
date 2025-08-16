/*
 * iocccsize - IOCCC Source Size Tool
 *
 *	"You are not expected to understand this" :-)
 *
 *	Public Domain 1992, 2022 by Anthony Howe.  All rights released.
 *	With IOCCC mods in 2019, 2022 by chongo (Landon Curt Noll) ^oo^
 *
 * SYNOPSIS
 *
 * 	iocccsize [-ihV][-v level] prog.c
 * 	iocccsize [-ihV][-v level] < prog.c
 *
 *	-i		ignored for backward compatibility
 *	-h		print usage message in stderr and exit
 *	-v level	turn on some debugging to stderr
 *	-V		print version and exit
 *
 *	The IOCCC net count rule 2b is written to stdout; with -v1, net count (2b),
 *	gross count (2a), number of keywords counted as 1 byte.  With -v2 or -v3
 *	write source to stdout and tool diagnostics to stderr.
 *
 * DESCRIPTION
 *
 *	Reading a C source file from standard input, apply the IOCCC
 *	source size rules as explained in the Guidelines.  The source
 *	code is passed through on standard output.  The source's net
 *	length is written to standard error; with -v option the net
 *	length, gross length, and matched keyword count are written.
 *
 *	The entry's gross size in bytes must be less than equal to 4993
 *	bytes in length.
 *
 *	The entry's net size in bytes must be less than equal to 2503
 *	bytes.  The net size is computed as follows:
 *
 *	The size tool counts most C reserved words (keyword, secondary,
 *	and selected preprocessor keywords) as 1.  The size tool counts all
 *	other octets as 1 excluding ASCII whitespace, and excluding any
 *	';', '{' or '}' followed by ASCII whitespace, and excluding any
 *	';', '{' or '}' octet immediately before the end of file.
 */

/* ISO C11 section 5.2.1 defines source character set, specifically:
 *
 *	The representation of each member of the source and execution
 *	basic character sets shall fit in a byte.
 *
 * Note however that string literals and comments could contain non-ASCII
 * (consider non-English developers writing native language comments):
 *
 *	If any other characters are encountered in a source file (except
 *	in an identifier, a character constant, a string literal, a header
 *	name, a comment, or a preprocessing token that is never converted
 *	to a token), the behavior is undefined.
 *
 * Probably best to leave as-is, count them, and let the compiler sort it.
 */
#undef ASCII_ONLY

/*
 * IOCCC Judge's remarks:
 *
 * This code contains undocumented features.  On the other hand, this code
 * is RTFS (for certain values of RTFS).  One might say that this code
 * perfectly documents itself.  :-)
 *
 * Many thanks to Anthony Howe for taking the time to put his OCD
 * (Obfuscated Coding Determination) into this code!
 */

/*
 * HINT: The algorithm implemented by this code may or not be obfuscated.
 *       The algorithm may not or may appear to be obfuscated.
 *
 * In particular:
 *
 *      We did not invent the algorithm.
 *      The algorithm consistently finds Obfuscation.
 *      The algorithm killed Obfuscation.
 *      The algorithm is banned in C.
 *      The algorithm is from Bell Labs in Jersey.
 *      The algorithm constantly finds Obfuscation.
 *      This is not the algorithm.
 *      This is close.
 */

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "iocccsize.h"

#define STRLEN(s)		(sizeof (s)-1)

#define NO_STRING		0
#define NO_COMMENT		0
#define COMMENT_EOL		1
#define COMMENT_BLOCK		2

int rule_count_debug;

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
	/* Yes Virginia, we left #define off the list on purpose!  K&R */
	{ STRLEN("#elif"), "#elif" } ,				/* K&R */
	{ STRLEN("#elifdef"), "#elifdef" } ,			/* +C23 */
	{ STRLEN("#elifndef"), "#elifndef" } ,			/* +C23 */
	{ STRLEN("#else"), "#else" } ,				/* K&R */
	{ STRLEN("#embed"), "#embed" } ,			/* +C23 */
	{ STRLEN("#endif"), "#endif" } ,			/* K&R */
	{ STRLEN("#error"), "#error" } ,			/* +C89 */
	{ STRLEN("#ident"), "#ident" } ,			/* gcc */
	{ STRLEN("#if"), "#if" } ,				/* K&R */
	{ STRLEN("#ifdef"), "#ifdef" } ,			/* K&R */
	{ STRLEN("#ifndef"), "#ifndef" } ,			/* K&R */
	{ STRLEN("#include"), "#include" } ,			/* K&R */
	{ STRLEN("#line"), "#line" } ,				/* K*R */
	{ STRLEN("#pragma"), "#pragma" } ,			/* +C89 */
	{ STRLEN("#sccs"), "#sccs" } ,				/* gcc */
	{ STRLEN("#warning"), "#warning" } ,			/* +C23 */
	{ STRLEN("#undef"), "#undef" } ,			/* K&R */

	{ STRLEN("_Alignas"), "_Alignas" } ,			/* +C11 */
	{ STRLEN("_Alignof"), "_Alignof" } ,			/* +C11 */
	{ STRLEN("_Atomic"), "_Atomic" } ,			/* +C11 */
	{ STRLEN("_BitInt"), "_BitInt" } ,			/* +C23 */
	{ STRLEN("_Bool"), "_Bool" } ,				/* +C99 */
	{ STRLEN("_Complex"), "_Complex" } ,			/* +C99 */
	{ STRLEN("_Decimal128"), "_Decimal128" } ,		/* +C23 */
	{ STRLEN("_Decimal64"), "_Decimal64" } ,		/* +C23 */
	{ STRLEN("_Decimal32"), "_Decimal32" } ,		/* +C23 */
	{ STRLEN("_Generic"), "_Generic" } ,			/* +C11 */
	{ STRLEN("_Imaginary"), "_Imaginary" } ,		/* +C99 */
	{ STRLEN("_Noreturn"), "_Noreturn" } ,			/* +C11 */
	{ STRLEN("_Pragma"), "_Pragma" } ,			/* +C99 */
	{ STRLEN("_Static_assert"), "_Static_assert" } ,	/* +C11 */
	{ STRLEN("_Thread_local"), "_Thread_local" } ,		/* +C11 */

	{ STRLEN("alignas"), "alignas" } ,			/* +C23 */
	{ STRLEN("alignof"), "alignof" } ,			/* +C23 */
	{ STRLEN("and"), "and" } ,				/* +C89 iso646.h */
	{ STRLEN("and_eq"), "and_eq" } ,			/* +C89 iso646.h */
	{ STRLEN("auto"), "auto" } ,				/* K&R */
	{ STRLEN("bitand"), "bitand" } ,			/* +C89 iso646.h */
	{ STRLEN("bitor"), "bitor" } ,				/* +C89 iso646.h */
	{ STRLEN("bool"), "bool" } ,				/* +C23 */
	{ STRLEN("break"), "break" } ,				/* K&R */
	{ STRLEN("case"), "case" } ,				/* K&R */
	{ STRLEN("char"), "char" } ,				/* K&R */
	{ STRLEN("compl"), "compl" } ,				/* +C89 iso646.h */
	{ STRLEN("const"), "const" } ,				/* +C89 */
	{ STRLEN("continue"), "continue" } ,			/* K&R */
	{ STRLEN("default"), "default" } ,			/* K&R */
	{ STRLEN("do"), "do" } ,				/* K&R */
	{ STRLEN("double"), "double" } ,			/* K&R */
	{ STRLEN("else"), "else" } ,				/* K&R */
	{ STRLEN("enum"), "enum" } ,				/* +C89 */
	{ STRLEN("extern"), "extern" } ,			/* K&R */
	{ STRLEN("false"), "false" } ,				/* +C23 */
	{ STRLEN("float"), "float" } ,				/* K&R */
	{ STRLEN("for"), "for" } ,				/* K&R */
	{ STRLEN("goto"), "goto" } ,				/* K&R */
	{ STRLEN("if"), "if" } ,				/* K&R */
	{ STRLEN("inline"), "inline" } ,			/* +C99 */
	{ STRLEN("int"), "int" } ,				/* K&R */
	{ STRLEN("long"), "long" } ,				/* K&R */
	{ STRLEN("noreturn"), "noreturn" } ,			/* +C23 */
	{ STRLEN("not"), "not" } ,				/* +C89 iso646.h */
	{ STRLEN("not_eq"), "not_eq" } ,			/* +C89 iso646.h */
	{ STRLEN("or"), "or" } ,				/* +C89 iso646.h */
	{ STRLEN("or_eq"), "or_eq" } ,				/* +C89 iso646.h */
	{ STRLEN("register"), "register" } ,			/* K&R */
	{ STRLEN("restrict"), "restrict" } ,			/* +C99 */
	{ STRLEN("return"), "return" } ,			/* K&R */
	{ STRLEN("short"), "short" } ,				/* K&R */
	{ STRLEN("signed"), "signed" } ,			/* K&R */
	{ STRLEN("sizeof"), "sizeof" } ,			/* K&R */
	{ STRLEN("static"), "static" } ,			/* K&R */
	{ STRLEN("static_assert"), "static_assert" } ,		/* +C23 */
	{ STRLEN("struct"), "struct" } ,			/* K&R */
	{ STRLEN("switch"), "switch" } ,			/* K&R */
	{ STRLEN("thread_local"), "thread_local" } ,		/* +C23 */
	{ STRLEN("true"), "true" } ,				/* +C23 */
	{ STRLEN("typedef"), "typedef" } ,			/* K&R */
	{ STRLEN("typeof"), "typeof" } ,			/* +C23 */
	{ STRLEN("typeof_unequal"), "typeof_unequal" } ,	/* +C23 */
	{ STRLEN("union"), "union" } ,				/* K&R */
	{ STRLEN("unsigned"), "unsigned" } ,			/* K&R */
	{ STRLEN("void"), "void" } ,				/* +C89 */
	{ STRLEN("volatile"), "volatile" } ,			/* +C89 */
	{ STRLEN("while"), "while" } ,				/* K&R */
	{ STRLEN("xor"), "xor" } ,				/* +C89 iso646.h */
	{ STRLEN("xor_eq"), "xor_eq" } ,			/* +C89 iso646.h */

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

RuleCount
rule_count(FILE *fp_in)
{
	size_t wordi = 0;
	char word[WORD_BUFFER_SIZE];
	RuleCount counts = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int ch, next_ch, quote = NO_STRING, escape = 0, is_comment = NO_COMMENT;

	/* More to silence Valgrind and be safe. */
	(void) memset(word, 0, sizeof (word));

/* If quote == NO_STRING (0) and is_comment == NO_COMMENT (0) then its code. */
#define IS_CODE	(quote == is_comment)

	while ((ch = fgetc(fp_in)) != EOF) {
		if (ch == '\r') {
			/* Discard bare CR and those part of CRLF. */
			counts.rule_2a_size++;
			continue;
		}
		if (ch == '\0') {
			counts.rule_2a_size++;
			counts.nul++;
			continue;
		}
#ifdef ASCII_ONLY
		if (128 <= ch) {
			counts.rule_2a_size++;
			counts.high_bit++;
			continue;
		}
#endif

		/* Future gazing. */
		while ((next_ch = fgetc(fp_in)) != EOF && next_ch == '\r') {
			/* Discard bare CR and those part of CRLF. */
			counts.rule_2a_size++;
		}
		if (next_ch == '\0') {
			(void) ungetc(ch, fp_in);
			counts.rule_2a_size++;
			counts.nul++;
			continue;
		}
#ifdef ASCII_ONLY
		if (128 <= next_ch) {
			(void) ungetc(ch, fp_in);
			counts.rule_2a_size++;
			counts.high_bit++;
			continue;
		}
#endif

#ifdef TRIGRAPHS
		if (ch == '?' && next_ch == '?') {
			/* ISO C11 section 5.2.1.1 Trigraph Sequences */
			const char *t;
			static const char trigraphs[] = "=#([)]'^<{!|>}-~/\\";

			ch = fgetc(fp_in);
			for (t = trigraphs; *t != '\0'; t += 2) {
				if (ch == t[0]) {
					/* Mapped trigraphs count as 1 byte. */
					next_ch = fgetc(fp_in);
					counts.rule_2a_size += 2;
					ch = t[1];
					break;
				}
			}

			/* Unknown trigraph, push back the 3rd character. */
			if (*t == '\0') {
				if (ch != EOF && ungetc(ch, fp_in) == EOF) {
					counts.ungetc_error++;
					/* Proceed with bad ch = '?'. */
				}
				counts.bad_trigraph++;
				ch = '?';
			}
		}
#endif
		if (ch == '\\' && next_ch == '\n') {
			/* ISO C11 section 5.1.1.2 Translation Phases
			 * point 2 discards backslash newlines.
			 */
			counts.rule_2a_size += 2;
			continue;
		}

		if (next_ch != EOF && ungetc(next_ch, fp_in) == EOF) {
			/* ISO C ungetc() guarantees one character (byte) pushback.
			 * How does that relate to UTF8 and wide-character library
			 * handling?  An invalid trigraph results in 2x ungetc().
			 */
			counts.ungetc_error++;
			counts.rule_2a_size++;
			continue;
		}

		/* Within quoted string? */
		if (quote != NO_STRING) {
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
				quote = NO_STRING;
			}
		}

		/* Within comment to end of line? */
		else if (is_comment == COMMENT_EOL && ch == '\n') {
			if (rule_count_debug > 1) {
				(void) fprintf(stderr, "~~NO_COMMENT\n");
			}
			is_comment = NO_COMMENT;
		}

		/* Within comment block? */
		else if (is_comment == COMMENT_BLOCK && ch == '*' && next_ch == '/') {
			if (rule_count_debug > 1) {
				(void) fprintf(stderr, "~~NO_COMMENT\n");
			}
			is_comment = NO_COMMENT;
		}

		/* Start of comment to end of line? */
		else if (is_comment == NO_COMMENT && ch == '/' && next_ch == '/') {
			if (rule_count_debug > 1) {
				(void) fprintf(stderr, "~~COMMENT_EOL\n");
			}
			is_comment = COMMENT_EOL;

			/* Consume next_ch. */
			if (rule_count_debug > 1) {
				(void) fputc(ch, stdout);
			}
			ch = fgetc(fp_in);
			counts.rule_2a_size++;
			counts.rule_2b_size++;
		}

		/* Start of comment block? */
		else if (is_comment == NO_COMMENT && ch == '/' && next_ch == '*') {
			if (rule_count_debug > 1) {
				(void) fprintf(stderr, "~~COMMENT_BLOCK\n");
			}
			is_comment = COMMENT_BLOCK;

			/* Consume next_ch. */
			if (rule_count_debug > 1) {
				(void) fputc(ch, stdout);
			}
			ch = fgetc(fp_in);
			counts.rule_2a_size++;
			counts.rule_2b_size++;
		}

		/* Open single or double quote? */
		else if (is_comment == NO_COMMENT && (ch == '\'' || ch == '"')) {
			quote = ch;
		}

		if (rule_count_debug > 1) {
			(void) fputc(ch, stdout);
		}

#ifdef DIGRAPHS
		/* ISO C11 section 6.4.6 Punctuators, digraphs handled during
		 * tokenization, but map here and count as 1 byte, like their
		 * ASCII counter parts.
		 */
		if (IS_CODE) {
			const char *d;
			static const char digraphs[] = "[<:]:>{<%}%>#%:";
			for (d = digraphs; *d != '\0'; d += 3) {
				if (ch == d[1] && next_ch == d[2]) {
					if (rule_count_debug > 1) {
						(void) fputc(next_ch, stdout);
					}
					(void) fgetc(fp_in);
					counts.rule_2a_size++;
					ch = d[0];
					break;
				}
			}
		}
#endif
		/* Sanity check against file size and wc(1) byte count. */
		counts.rule_2a_size++;

		/* End of possible keyword?  Care with #word as there can
		 * be whitespace or comments between # and word.
		 */
		if ((word[0] != '#' || 1 < wordi) && !isalnum(ch) && ch != '_' && ch != '#') {
			if (find_member(cwords, word) != NULL) {
				/* Count keyword as 1. */
				counts.rule_2b_size = counts.rule_2b_size - wordi + 1;
				counts.keywords++;
				if (rule_count_debug > 1) {
					(void) fprintf(stderr, "~~keyword %zu \"%s\"\n", counts.keywords, word);
				}
			}
			word[wordi = 0] = '\0';
		}

		/* Ignore all whitespace. */
		if (isspace(ch)) {
			if (rule_count_debug > 2) {
				(void) fprintf(stderr, "~~ignore whitespace %#02x\n", ch);
			}
			continue;
		}

		/* Ignore begin/end block and end of statement. */
		if (strchr("{;}", ch) != NULL && (isspace(next_ch) || next_ch == EOF)) {
			if (rule_count_debug > 2) {
				(void) fprintf(stderr, "~~ignore %c\n", ch);
			}
			continue;
		}

		/* Collect next word not in a string or comment. */
		if (IS_CODE && (isalnum(ch) || ch == '_' || ch == '#')) {
			word[wordi++] = (char) ch;
			if (sizeof (word) <= wordi) {
				/* ISO C11 section 5.2.4.1 Translation limits, identifiers
				 * can have 63 significant initial characters, which can be
				 * multibyte.  The C keywords are all ASCII, longest is 14
				 * bytes.
				 *
				 * We only care about the C keywords and not the identifiers,
				 * so the buffer can overflow regularly as long words or
				 * identifiers are ignored.
				 */
				if (rule_count_debug > 1) {
					(void) fprintf(stderr, "~~word buffer %*s\n", (int) sizeof (word), word);
				}
				counts.word_overflow++;
				wordi = 0;
			}
			word[wordi] = '\0';
		}

		counts.rule_2b_size++;
	}

	return counts;
}

#ifdef WITH_MAIN

static char *out_fmt = "%lu\n";

static char usage[] =
"usage: iocccsize [-ihV][-v level] prog.c\n"
"       iocccsize [-ihV][-v level] < prog.c\n"
"\n"
"-i\t\tignored for backward compatibility\n"
"-h\t\tprint usage message in stderr and exit\n"
"-v level\tturn on some debugging to stderr\n"
"-V\t\tprint version and exit\n"
"\n"
"The IOCCC net count rule 2b is written to stdout; with -v1, net count (2b),\n"
"gross count (2a), number of keywords counted as 1 byte.  With -v2 or -v3\n"
"write source to stdout and tool diagnostics to stderr.\n"
;

static char exits[] =
"\n"
"Exit Codes\n"
"\n"
"0\tsource code passes rules 2a and 2b.\n"
"1\tsource code fails rule 2a and/or 2b.\n"
"2\tusage -h\n"
"3\tversion -V\n"
"4\tcommand line argument error\n"
"5+\tsome other internal error\n"
;

int
main(int argc, char **argv)
{
	int ch;
	char *stop;
	RuleCount counts;
	FILE *fp_in = stdin;

	while ((ch = getopt(argc, argv, "6ihrv:V")) != -1) {
		switch (ch) {
		case 'i': /* ignored for backward compatibility */
			break;

		case 'v':
			rule_count_debug = (int) strtol(optarg, &stop, 0);
			if (*stop != '\0') {
				(void) fprintf(stderr, "bad -v argument: %s\n", optarg);
				exit(4); /*ooo*/
			}
			out_fmt = "%lu %lu %lu\n";
			break;

		case 'V':
			(void) printf("%s\n", VERSION);
			exit(3); /*ooo*/

		case '6': /* You're a RTFS master!  Congrats. */
			(void) fprintf(stderr, "There is NO... Rule 6!  I'm not a number!  I'm a free(void *man)!\n");
			exit(6); /*ooo*/

		case 'h':
		default:
			(void) fprintf(stderr, "%s%s\n", usage, exits);
			(void) fprintf(stderr, "Rule size 2a <= %u, 2b <= %u\n", RULE_2A_SIZE, RULE_2B_SIZE);
			exit(2); /*ooo*/
		}
	}

	if (optind + 1 == argc) {
		/* Redirect stdin to file path argument. */
		if ((fp_in = fopen(argv[optind], "r")) == NULL) {
			(void) fprintf(stderr, "%s: %s\n", argv[optind], strerror(errno));
			exit(6); /*ooo*/
		}
	} else if (optind != argc) {
		/* Too many arguments. */
		(void) fprintf(stderr, "%s%s\n", usage, exits);
		exit(2); /*ooo*/
	}

	(void) setvbuf(fp_in, NULL, _IOLBF, 0);
	(void) setvbuf(stdout, NULL, _IOLBF, 0);

	/* The Count - 1 Muha .. 2 Muhaha .. 3 Muhahaha ... */
	counts = rule_count(fp_in);

	/* Any warnings? */
	if (0 < counts.nul) {
		(void) fprintf(stderr, "warning: %lu NUL bytes seen; careful not to violate rule 13!\n", counts.nul);
	}
	if (0 < counts.high_bit) {
		(void) fprintf(stderr, "warning: %lu non-ASCII bytes; parlez vous Francais?\n", counts.high_bit);
	}
	if (0 < counts.bad_trigraph) {
		(void) fprintf(stderr, "warning: %lu bad trigraphs; is that a bug or feature of your code?\n", counts.bad_trigraph);
	}
	if (0 < counts.ungetc_error) {
		(void) fprintf(stderr, "warning: %lu ungetc errors; @SirWumpus goofed. The count on stdout may be invalid under rule 2!\n", counts.ungetc_error);
	}
	if (0 < counts.word_overflow) {
		(void) fprintf(stderr, "warning: %lu word buffer overflows; is that a bug or feature of your code?\n", counts.word_overflow);
	}
	if (RULE_2A_SIZE < counts.rule_2a_size) {
		(void) fprintf(stderr, "warning: size %lu exceeds Rule 2a %u\n", counts.rule_2a_size, RULE_2A_SIZE);
	}
	if (RULE_2B_SIZE < counts.rule_2b_size) {
		(void) fprintf(stderr, "warning: count %lu exceeds Rule 2b %u\n", counts.rule_2b_size, RULE_2B_SIZE);
	}

	(void) printf(out_fmt, counts.rule_2b_size, counts.rule_2a_size, counts.keywords);

	/*
	 * All Done!!! All Done!!! -- Jessica Noll, age 2
	 */
	if ((RULE_2A_SIZE < counts.rule_2a_size) || (RULE_2B_SIZE < counts.rule_2b_size)) {
		exit(1); /*ooo*/
	}

	exit(0); /*ooo*/
}

#endif /* WITH_MAIN */
