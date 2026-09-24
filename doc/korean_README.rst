Experimental Korean stemmer
==========================

This is a deliberately limited, word-at-a-time Snowball implementation.
The algorithm is implemented once in ``algorithms/korean.sbl``; the existing
generators produce the target-language implementations. There is no separate
Python stemming algorithm or external morphological-analysis dependency.

Input and output
----------------

Pass one NFC-normalized Korean word at a time. The algorithm does not split
sentences, infer parts of speech from context, or normalize Unicode. The
libstemmer algorithm name is ``korean``, with aliases ``ko`` and ``kor``.
Its result follows the existing libstemmer buffer ownership and length API.

Examples::

    Input       Output
    학생이      학생
    사과를      사과
    먹었다      먹다
    공부했다    공부하다
    고양이      고양이
    걸었다      걸었다

Why the rules are conservative
-----------------------------

Removing a final 이, 가, 을 or 를 from every matching string would damage
ordinary words such as 고양이, 국가, 사과 and 마을. The noun routine therefore
accepts only explicitly listed base-plus-particle paths, with the appropriate
particle allomorph for the base. The initial set contains 24 noun/pronoun bases.
An unlisted name such as 철수가 is preserved instead of guessed.

The verb routine maps explicitly enumerated inflections for 13 verb/adjective
lemmas to their dictionary forms. It includes selected regular, irregular and
contracted forms, but it does not implement the full Korean conjugation system.
The complete accepted forms are visible in the source. For example, 먹었다
is supported, while 먹는다 is not currently normalized.

Some surface forms require sentence context to disambiguate. 걸었다 can refer
to walking or hanging/calling, and 들었다 can refer to hearing or lifting.
These forms are preserved. The ambiguous form 나는 and pronoun contractions
내가, 제가, 네가 and 누가 are also preserved by this initial policy.

The routines require a complete word match, preventing a recognized suffix
from silently changing an unrecognized word. Other particles, particle chains,
unlisted inflections and unchanged words are preserved. Broad recall is not
claimed: this is a small experimental baseline for review, rather than complete
Korean stemming or a part-of-speech analyzer.

Testing
-------

The hand-authored regression vocabulary has 190 input/output pairs covering
supported forms and preservation cases. Its license is in
``tests/korean/data/korean/COPYING``. Run::

    make -j4 check_korean python=python3

This compares generated C, Python and JavaScript implementations against the
same vocabulary. It needs the existing compiler tools, Python and Node.js;
it does not install Python packages or a language model.

The general CI and coverage workflows use this vocabulary only if their
snowball-data checkout does not already provide a ``korean`` directory. A
matching snowball-data branch takes precedence. A larger licensed vocabulary
and algorithm website documentation remain companion upstream contributions
as described in ``CONTRIBUTING.rst``; the local cases are regression tests,
not a general-language accuracy benchmark.
