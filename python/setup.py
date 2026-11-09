#!/usr/bin/env python

from setuptools import setup
import re

SNOWBALL_VERSION = '3.2.0'

n_stemmers = 0

langs = []
variants = {}
with open('modules.txt') as fp:
    for line in fp.readlines():
        if len(line) <= 1 or line[0] == '#':
            continue
        if line[-1:] == '\n':
            line = line[:-1]
        tokens = re.split(r'\s+', line)
        if len(tokens) < 3:
            print("Bad modules.txt line: " + line)
            continue
        (name, encs, codes) = tokens[:3]
        if len(tokens) > 3:
            variant_of = tokens[3]
            if variant_of in variants:
                variants[variant_of].append(name)
            else:
                variants[variant_of] = [name]
        else:
            langs.append(name)
        n_stemmers += 1

desc = 'This package provides ' + str(n_stemmers) + ' stemmers for ' + \
    str(len(langs)) + ' languages generated from Snowball algorithms.'

classifiers = [
    'Development Status :: 5 - Production/Stable',
    'Intended Audience :: Developers',
]

for lang in langs:
    # Only classifiers listed in https://pypi.org/classifiers/ are allowed
    # Remove them here or submit them to https://github.com/pypa/trove-classifiers
    if lang in (
            'earlymodernenglish',
            'sesotho',
            ):
        continue

    lang_titlecase = lang.title()
    classifiers.append('Natural Language :: ' + lang_titlecase)

    # Portuguese stemmer also works for Brazilian Portuguese.
    if lang_titlecase == 'Portuguese':
        classifiers.append('Natural Language :: Portuguese (Brazilian)')

classifiers.extend([
    'Operating System :: OS Independent',
    'Programming Language :: Python',
    'Programming Language :: Python :: 3'
])

# NB adds minor versions < range end.
for minor in range(3, 15 + 1):
    classifiers.append('Programming Language :: Python :: 3.%d' % minor)

classifiers.extend([
    'Programming Language :: Python :: Implementation :: CPython',
    'Programming Language :: Python :: Implementation :: PyPy',
    'Topic :: Database',
    'Topic :: Internet :: WWW/HTTP :: Indexing/Search',
    'Topic :: Text Processing :: Indexing',
    'Topic :: Text Processing :: Linguistic'
])

setup(name='snowballstemmer',
      version=SNOWBALL_VERSION,
      description=desc,
      author='Snowball Developers',
      author_email='snowball-discuss@lists.tartarus.org',
      url='https://github.com/snowballstem/snowball',
      keywords="stemmer",
      license="BSD-3-Clause",
      packages=['snowballstemmer'],
      package_dir={"snowballstemmer": "src/snowballstemmer"},
      python_requires='>=3.3',
      classifiers = classifiers
)
