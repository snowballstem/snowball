Snowball is a small string processing language for creating stemming algorithms
for use in Information Retrieval, plus a collection of stemming algorithms
implemented using it.

Snowball was originally designed and built by Martin Porter.  Martin retired
from development in 2014 and Snowball is now maintained as a community project.
Martin originally chose the name Snowball as a tribute to SNOBOL, the excellent
string handling language from the 1960s.  It now also serves as a metaphor for
how the project grows by gathering contributions over time.

The Snowball compiler translates a Snowball program into source code in another
language - currently Ada, ISO C, C#, Go, Java, Javascript, Object Pascal,
Python and Rust are supported.

This repository contains the source code for the snowball compiler and the
stemming algorithms.  The snowball compiler is written in ISO C - you'll need
a C compiler which support C99 to build it (but the C code it generates should
work with any ISO C compiler).

See https://snowballstem.org/ for more information about Snowball.

What is Stemming?
=================

Stemming maps different forms of the same word to a common "stem" - for
example, the English stemmer maps *connection*, *connections*, *connective*,
*connected*, and *connecting* to *connect*.  So a search for *connected*
would also find documents which only have the other forms.

This stem form is often a word itself, but this is not always the case as this
is not a requirement for text search systems, which are the intended field of
use.  We also aim to conflate words with the same meaning, rather than all
words with a common linguistic root (so *awe* and *awful* don't have the same
stem), and over-stemming is more problematic than under-stemming so we tend not
to stem in cases that are hard to resolve.  If you want to always reduce words
to a root form and/or get a root form which is itself a word then Snowball's
stemming algorithms likely aren't the right answer.

Building Snowball
=================

GNU make is required to build Snowball.

The build system is currently structured as two separate stages for many of the
target languages.

The first stage builds the Snowball compiler and runs it to create target
language code (and it can also run tests on each stemmer).  The expectation is
that you then create a "distribution" tarballs of this code with ``make dist``
(or to create one for a specific target language, e.g.  ``make
dist_libstemmer_c`` for C).  These tarballs are created in the ``dist/``
subdirectory.

To actually build the libstemmer library you then unpack and build the
distribution tarball, e.g. for C::

    tar xf dist/libstemmer_c-3.0.0.tar.gz
    cd libstemmer_c-3.0.0
    make

Cross-compiling
---------------

If cross-compiling starting from the git repo, the Snowball compiler needs to
be built with a native compiler then libstemmer with the cross-compiler.  For
example::

    make CC=cc dist_libstemmer_c
    tar xf dist/libstemmer_c-3.0.0.tar.gz
    cd libstemmer_c-3.0.0
    make CC=riscv64-unknown-linux-gnu-gcc

If you are cross-compiling to or from Microsoft Windows, you'll need to also
work around an assumption in libstemmer's ``Makefile`` which sets ``EXEEXT``
based on the OS you are building on::

    ifeq ($(OS),Windows_NT)
    EXEEXT=.exe
    endif

For example, if cross-compiling from Linux to Microsoft Windows, use something
like this for the libstemmer build::

    make CC=x86_64-w64-mingw32-gcc EXEEXT=.exe

When going the other way, you'll need to use ``EXEEXT=``.
