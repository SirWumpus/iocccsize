iocccsize
=========
Primary Repository https://codeberg.org/SirWumpus/iocccsize

The original IOCCC source code rule size checking tool.

```
usage: iocccsize [-ihV][-g size][-n size][-v level] prog.c
       iocccsize [-ihV][-g size][-n size][-v level] < prog.c

-i              ignored for backward compatibility
-g size         rule 2a gross size; default 4993
-h              print usage message in stderr and exit
-n size         rule 2b net size; default 2503
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


History
-------

Following my [IOCCC 1991 Best Utility](https://github.com/SirWumpus/ioccc-ae) win I needed something to help with applying the IOCCC Rule 2 Size Limit during the development of my [1992](https://github.com/SirWumpus/ioccc-am) and [1993](https://github.com/SirWumpus/ioccc-am) submissions (combined with some `sed(1)` transformations).  The tool kept being refined over the years as I kepted submitted entries to the IOCCC.

Sometime around 2012 or 2013 I was in contact with one of the IOCCC judges, [Landon Court Noll](https://github.com/lcn2), proposing  Rule 2 Size Limit changes and mentioned I had this tool I used to help prepare my entries.  Landon perked up to the idea of including the tool as part of the contest to help contestants (and judges) apply the Rule 2 Size Limit.  The `iocccsize` tool first appeared in IOCCC 2014.

The IOCCC has now moved to retool its submission process by integrating a version of this tool into [mkiocccentry](https://github.com/ioccc-src/mkiocccentry) with changes made to turn the core into a library function `rule_count()`.
