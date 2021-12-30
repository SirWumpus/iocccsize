iocccsize
=========

The IOCCC source code rule size checking tool.

```
usage: iocccsize [-ihvV] prog.c
       iocccsize [-ihvV] < prog.c

-i              ignored for backward compatibility
-h              print usage message in stderr and exit
-v              turn on some debugging to stderr; -vv or -vvv for more
-V              print version and exit

The source is written to stdout, with possible translations ie. trigraphs.
The IOCCC net count rule 2b is written to stderr; with -v, net count (2b),
gross count (2a), number of keywords counted as 1 byte; -vv or -vvv write
more tool diagnostics.
```
