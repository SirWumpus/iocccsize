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

#ifndef MAX_SIZE
#define MAX_SIZE		4096	/* IOCCC Rule 2a */
#endif

#ifndef MAX_COUNT
#define MAX_COUNT		2053	/* IOCCC Rule 2b */
#endif

typedef struct {
	size_t net;
	size_t gross;
	size_t keywords;
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
