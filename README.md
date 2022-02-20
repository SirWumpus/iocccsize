iocccsize
=========

The IOCCC source code rule size checking tool.

```sh
usage: iocccsize [-h] [-i] [-v level] [-V] prog.c
usage: iocccsize [-h] [-i] [-v level] [-V] < prog.c

-i		ignored for backward compatibility
-h		print usage message in stderr and exit 2
-v level	set debug level (def: none)
-V		print version and exit 3

	By default,the Rule 2b count is written to stdout.
	If the debug level is > 0, then the Rule 2a, Rule 2b,
	and keyword count is written to stdout instead.

Exit codes:
	0 - source code is within Rule 2a and Rule 2b limits
	1 - source code larger than Rule 2a and/or Rule 2b limits
	2 - -h used and help printed
	3 - -V used and version printed
	4 - invalid command line
	>= 5 - some internal error occurred
```
