/*
 * iocccsize - IOCCC Source Size Tool
 *
 * Public Domain 1992, 2022 by Anthony Howe.  All rights released.
 * With IOCCC minor mods in 2019, 2022 by chongo (Landon Curt Noll) ^oo^
 */

#ifndef __iocccsize_h__
#define __iocccsize_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* IOCCC tool chain, see https://github.com/ioccc-src/mkiocccentry */
#if defined(MKIOCCCENTRY_USE)
# include "limit_ioccc.h"		/* Can override our defines. */
#else
# define DIGRAPHS			/* Digraphs count as 1 for Rule 2b.*/
# define TRIGRAPHS			/* Trigraphs count as 1 for Rule 2b.*/
#endif

/* Official contest VERSION defined by limit_ioccc.h,
 * else the official git repository version tag.
 */
#ifndef VERSION
# define VERSION "unofficial"
#endif

#ifndef WORD_BUFFER_SIZE
# define WORD_BUFFER_SIZE	16
#endif

#ifndef RULE_2A_SIZE
# define RULE_2A_SIZE		4993	/* IOCCC Rule 2a, pre-2024 4096 */
#endif

#ifndef RULE_2B_SIZE
# define RULE_2B_SIZE		2503	/* IOCCC Rule 2b, pre-2024 2053 */
#endif

typedef struct {
	unsigned long rule_2a_size;	/* Bytes */
	unsigned long rule_2b_size;	/* Bytes */
	unsigned long rule_2a_chsize;	/* Characters */
	unsigned long rule_2b_chsize;	/* Characters */
	unsigned long keywords;		/* keyword count - for -v mode */
	unsigned long nul;		/* Count NUL bytes seen. */
	unsigned long bad_trigraph;	/* Count unknown trigraphs seen. */
	unsigned long ungetc_error;	/* Count ungetc depth exceeded. */
	unsigned long word_overflow;	/* Count word buffer overflows. */
} RuleCount;

/**
 * Set greater than zero for extra debug output.
 *
 *	0	no debug
 *	1	no debug assigned at this time
 *	2	source to standard output, comment & keyword debug to standard error.
 *	3	same a 2 with extra character classification debug
 */
extern int rule_count_debug;

/**
 * @param fp_in
 *	File pointer to the C source input.
 *
 * @return
 *	Return a RuleCount structure.
 */
extern RuleCount rule_count(FILE *fp_in);

#ifdef  __cplusplus
}
#endif

#endif /* __iocccsize_h__ */
