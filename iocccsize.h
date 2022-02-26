/*
 * iocccsize - IOCCC Source Size Tool
 *
 * Public Domain 1992, 2015, 2018, 2019, 2021, 2022 by Anthony Howe.  All rights released.
 */

#ifndef __iocccsize_h__
#define __iocccsize_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifndef VERSION
#define VERSION "28.3 2021-12-29"	/* use format: major.minor YYYY-MM-DD */
#endif

#ifndef WORD_BUFFER_SIZE
#define WORD_BUFFER_SIZE	64
#endif

#ifndef RULE_2A_SIZE
#define RULE_2A_SIZE		4096	/* IOCCC Rule 2a */
#endif

#ifndef RULE_2B_SIZE
#define RULE_2B_SIZE		2053	/* IOCCC Rule 2b */
#endif

typedef struct {
	size_t rule_2a_size;
	size_t rule_2b_size;
	size_t keywords;	/* keyword count - for -v mode */
} RuleCount;

/**
 * @param fp_in
 *	File pointer to the C source input.
 *
 * @param fp_out
 *	File pointer where to write the translated C source output.
 *	Can be NULL if no output is to be written.
 *
 * @return
 *	Return a RuleCount structure.
 */
extern RuleCount rule_count(FILE *fp_in, FILE *fp_out);

#ifdef  __cplusplus
}
#endif

#endif /* __iocccsize_h__ */
