Adding a new stemming algorithm
=============================

This needs PRs against three repositories.  Name the branch the same for
at least `snowball` and `snowball-data` and the CI should use your new
vocabularies lists when running the testsuite.

snowball repo
-------------

Add `.sbl` source to algorithms subdirectory.

Add entry to `libstemmer/modules.txt`, maintaining the current sorted order by
the first column.  The columns are:

* Algorithm name (needs to match the `.sbl` source without extension)
* Encodings to support
* Names and ISO codes for the language

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
language the stemmer is for.  If the language uses a script/alphabet which
isn't already supported you may need to add a regular new regular expression.

snowball-website repo
---------------------

Create subdirectory of `algorithms/` named after the language.

Create `stemmer.tt` which describes the stemming algorithm.  This is a
"template toolkit" template which is essentially a mix of HTML and some
macros for adding the navigation, sample vocabulary, etc.  See the
existing `stemmer.tt` files for other algorithms for inspiration.

If it is based on an academic paper, cite the paper and describe any difference
between your implementation and that described in the paper (for example,
sometimes papers have ambiguities that need resolving to re-implement the
algorithm described).

If you have a stopword list, add that as `stop.txt` and link to it from
`stemmer.tt`.

Link to your new `stemmer.tt` from `algorithms/index.tt`.

Add a news entry to `index.tt`.

.. FIXME: Also needs adding for the online demo.

Adding a new programming language backend
=========================================

Copy an existing `compiler/generator_*.c` for your new language and modify
away (`generator.c` has the generator for C, but also some common functions
so if you start from this one you'll need to remove those common functions).
Please resist reformatting existing code - there's currently a lot of code
repeated in each generator which ought to be pulled out as common code, and
if you reformat that just makes that job harder.

Add your new source to `COMPILER_SOURCES` in `GNUmakefile`.

Add prototypes for the new functions to `compiler/header.h`.

Add support to `compiler/driver.c`.

Add targets to `GNUmakefile` to run tests for the new language.

Hook up automated testing via CI in `.travis.yml`.
