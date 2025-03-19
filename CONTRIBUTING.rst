General contribution guidelines
===============================

We don't have a formally defined coding style guide, but please strive to
make new/changed code look like the code around it.

Use spaces-only for indentation except where there's a syntax reason (e.g.
``GNUmakefile``) or a strong convention (e.g. Go's standard seems to be tabs,
and ``gofmt`` reindents code using tabs).

Avoid adding trailing whitespace on lines.  Make sure there's a newline
character at the end of new text files.

Avoid mixing code reformatting changes with functional changes - doing so
makes it harder to review patches.

Adding a new stemming algorithm
===============================

To add a new stemming algorithm you need to submit PRs against three
repositories.  See below for details of what's needed in each of
these.

Name the branch the same for at least `snowball` and `snowball-data` and push
to `snowball-data` first, then the CI should use your new vocabulary list when
running the testsuite.

snowball repo
-------------

This is where the implementation of the new algorithm goes.  Add the `.sbl`
source implementing it to the `algorithms/` subdirectory.

Add entry to `libstemmer/modules.txt`, maintaining the current sorted order by
the first column.  The columns are:

* Algorithm name (needs to match the `.sbl` source without extension)
* Encodings to support.  Wide-character Unicode is always supported
  and doesn't need to be listed here.  You should always include `UTF_8`, and
  also any of `ISO_8859_1`, `ISO_8859_2` and `KOI8_R` which the language can
  usefully be written using only characters from (in particular they need to
  contain all the characters the stemmer explicitly uses).  Support for other
  single-byte character sets is easy to add if they're useful.
* Names and ISO-639 codes for the language.  Wikipedia has a handy list of `all
  the ISO-639 codes <https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes>`_ -
  find the row for your new language and include the codes from the "639-1",
  "639-2/T" and (if different) "639-2/B" columns.  For example, for the `Afar`
  language you'd put `afar,aa,aar` here.

Some points to note about algorithm implementations:

* Avoid literal non-ASCII characters in snowball string literals - they will
  work OK for languages that use UTF-8, but not wide-character Unicode or other
  encodings.  Instead use ``stringdef`` like the existing stemmers do, and
  please use the newer `U+` notation rather than the older ``hex`` or
  ``decimal`` as this allows us to support different encodings without having
  to modify the source files - for example::

    stringdef o" {U+00F6}
    define foo 'o{o"}'

  not::

    stringdef o" hex F6
    define foo 'o{o"}'

  and definitely not::

    define foo 'oö'

  It's OK to use UTF-8 in comments.

* It's helpful to consistently use the same ``stringdef`` codes across the
  different stemmers - for languages using the latin alphabet our website has
  `guidance on what to use <https://snowballstem.org/codesets/guide.html>`_ and
  a `list of stringdef lines for common characters to cut and paste from
  <https://snowballstem.org/codesets/latin-stringdef-list.txt>`_.

snowball-data repo
------------------

Add subdirectory named after new stemmer containing:

* voc.txt - word list
* output.txt - stemmed equivalents
* COPYING - licensing details (word lists need to be under an OSI-approved
  licence)

If you don't have access to a suitably licensed word list of a suitable size,
you may be able to use the `wikipedia-most-common-words` script to generate
one by extracting the most frequent words from a Wikipedia dump in the
language the stemmer is for.  You need to specify the Unicode "script" (that's
"script" in the sense of alphabet) to use - you can find the appropriate one
by looking in the Unicode `Scripts.txt
<https://www.unicode.org/Public/13.0.0/ucd/Scripts.txt>`_.

The script name is the second column, between `;` and `#`.  The first entries
are all "Common" which isn't what you want - scroll down to get to the entries
that are useful here.

You also need to specify the minimum frequency to select.  Picking this value
will probably need some experimentation as the appropriate threshold depends on
how much data there is in the wikipedia dump for a particular language, as well
as the size of the vocabulary for the language, and how inflected the language
is.  Try counting the number of unique words extracted (`wc -l voc.txt` on
Unix) and also looking through the list - some proper nouns, words from other
languages, typos, etc are OK (since the stemmer will encounter all these in
practice too), but at some point "more" stops being "better".

snowball-website repo
---------------------

This is where a description of the new algorithm goes.  Experience from
maintaining Snowball for many years has shown us that the most important
points to cover are **WHY** particular things are done or are not done.

For example, if a particular ending isn't removed because doing so causes
problems in other cases it's really helpful to have that recorded.  Then
if years later we get a bug report because this ending isn't removed we
can easily answer, and don't have to try to contact you and hope you can
remember, or try to work out why for ourselves.

The original set of Snowball stemmers each have an English prose description
of the algorithm which focuses on **WHAT** the algorithm does.  These might be
helpful if you want to implement the algorithm from scratch in a separate
language, but they've not proved very useful for maintaining the Snowball
implementations - if the prose and Snowball code disagree we know something is
wrong, but it's hard to know which is right!  Therefore we recommend to let
the Snowball implementation describe what the algorithm does, and only comment
on "**WHAT**" in cases where the implementation needs explanation to help
the reader understand it.

If your algorithm is based on an academic paper, cite the paper and describe
any differences between your implementation and that described in the paper.
For example, sometimes papers have ambiguities that need resolving to
re-implement the algorithm described - see the `Hindi
<https://snowballstem.org/algorithms/hindi/stemmer.html>`_ and `Indonesian
<https://snowballstem.org/algorithms/indonesian/stemmer.html>`_
stemming algorithms descriptions for examples.

The mechanics of adding the algorithm description are:

* Create subdirectory of `algorithms/` named after the language.

* Create `stemmer.tt` which describes the stemming algorithm.  This is a
  "template toolkit" template which is essentially a mix of HTML and some
  macros for adding the navigation, sample vocabulary, etc.  See the
  existing `stemmer.tt` files for other algorithms for how to use these
  macros.

* If you have a stopword list, add that as `stop.txt` in your new subdirectory.
  The `generate` script checks if such a file exists and if it does a link to
  it is automatically added.

* Link to your new `stemmer.tt` from `algorithms/index.tt`.

* Add a news entry to `index.tt`.

* Add the new stemmer to the online demo.  Assuming you have checkouts of the
  `snowball`, `snowball-data` and `snowball-website` repos in sibling
  directories:

  * run `make check_js` in the `snowball` repo
  * run `./update-js`
  * add the new stemmer to git with: `git add js/*-stemmer.js`
  * if the new language is written right-to-left (RTL) then add it to the check
    in `demo.tt` (search for `rtl` to find the place to change.)
  * `git commit`.

Adding a new programming language generator
===========================================

This is a short guide to adding support for generating code for another
programming language.

Is a new generator the right solution?
--------------------------------------

Adding a new code generator is probably not your only option if you want
to use Snowball from another language - most languages have support for
writing bindings to a C library, so this is probably another option.

Generating code can have advantages.  For example, it can be simpler to
deploy without C bindings which need to be built for a specific platform.

However, it's likely to be significantly more work to implement a new generator
than to write bindings to the generated C code, especially as the libstemmer
C API is a very small and simple one.  Generated code can also be slower -
currently the Snowball compiler often generates code that assumes an optimising
compiler will clean up redundant constructs, which is not a problem for C, and
probably not for most compiled languages, but for a language like Python C
bindings are much faster than the generated Python code (using pypy helps a
lot, but is still slower).  See doc/libstemmer_python_README for some timings.

That said, the unoptimised generated code has improved over time, and is likely
to improve further in the future.

Key problems to solve
---------------------

* You need to work out how to map the required flow of control in response
  to Snowball signals.

  In the generated C code this is mostly done using `goto`.  If your language
  doesn't provide an equivalent to `goto` then you'll need an alternative
  solution.

  In Java and JavaScript we use labelled `break` from blocks and loops
  instead.  If your language has an equivalent to this feature, that will
  probably work.

  For Python, we currently generate a `try:` ... `raise lab123` ...
  `except lab123: pass` construct.  This works, but doesn't seem ideal.

  If one of the mechanisms above sounds suitable then take a look at the
  generator for the respective generated output and generator code.  If
  not, come and talk to us on the snowball-discuss mailing list.

* Snowball's division is specified as integer division with semantics
  matching C - i.e. the result should be truncated (rounded towards zero).
  Some languages lack a built-in integer division operation, or have one
  which instead implements rounding towards negative infinity.  Existing
  backends with special handling here which may be useful to look at
  include Javascript, Pascal and Python.

Don't hardcode algorithm names
------------------------------

We want to avoid hard-coded lists of algorithms in the language-specific code
that have to be manually updated each time a new algorithm is added, because
that adds some extra tedious work for adding a new algorithm, and mechanical
updates done by hand tend to miss places that need updating, or code gets
copied and pasted from an existing case but not fully updated.

All the existing language backends generate any such code at build time, and
adding a new algorithm just requires updating `libstemmer/modules.txt`.

You can probably copy the approach used for Pascal (script `pascal/generate.pl`
works from template `stemwords-template.dpr` which has marked blocks of code
that get expanded for each stemming algorithm with a placeholder replaced by
the algorithm name.  For an alternative approach, see Rust where this is done
by `rust/build.rs`.

Mechanics of adding a new generator
-----------------------------------

Copy an existing `compiler/generator_*.c` for your new language and modify
away (`generator.c` has the generator for C, but also some common functions
so if you start from this one you'll need to remove those common functions).

Please resist reformatting existing C code - there's currently a lot of code
repeated in each generator which ought to be pulled out as common code, and
if you reformat that just makes that job harder.

Add your new source to `COMPILER_SOURCES` in `GNUmakefile`.

Add prototypes for the new functions to `compiler/header.h`.

Add support to `compiler/driver.c`.

Add targets to `GNUmakefile` to run tests for the new language.

Hook up automated testing via CI in `.github/workflows/ci.yml`.

Add to the list of languages in `README.rst`.
