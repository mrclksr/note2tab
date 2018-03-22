# note2tab

### About
note2tab translates single notes and chords into guitar tablature notation.
A note is given by an optional accidental (#, b, or the natural sign %) and a
vertical numeric position on the staff. Position 1 is the undermost line,
position 2 is the space above the undermost line, and so forth. Position 0 is
the space below the undermost line, position -1 is the first ledger line below
position 0, and so forth. A chord is given by two or more different notes
enclosed in (). Example: (-1 b1 b3)

### Installation
The installation path for the executable is ${PREFIX}/bin, and the path for
the manpage is ${PREFIX}/man/man1. The default for ${PREFIX} is /usr/local.
~~~
# make PREFIX=/where/ever/you/want install
~~~

