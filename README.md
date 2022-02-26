iocccsize
=========

The IOCCC source code rule size checking tool.

```
usage: iocccsize [-ihV][-v level] prog.c
       iocccsize [-ihV][-v level] < prog.c

-i              ignored for backward compatibility
-h              print usage message in stderr and exit
-v level        turn on some debugging to stderr
-V              print version and exit

The IOCCC net count rule 2b is written to stdout; with -v1, net count (2b),
gross count (2a), number of keywords counted as 1 byte.  With -v2 or -v3
write source to stdout and tool diagnostics to stderr.

Exit Codes

0       source code passes rules 2a and 2b.
1       source code fails rule 2a and/or 2b.
2       usage -h
3       version -V
4       command line argument error
5+      some other internal error
```
