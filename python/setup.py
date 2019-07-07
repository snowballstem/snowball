#!/usr/bin/env python

from distutils.core import setup
import re

n_stemmers = 0

langs = []
variants = {}
for line in open('modules.txt').readlines():
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

long_desc = '''
It includes following language algorithms:

'''

classifiers = [
    'Development Status :: 5 - Production/Stable',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: BSD License'
]

for lang in langs:
    lang_titlecase = lang.title()
    long_desc += '* ' + lang_titlecase
    # Only classifiers listed in https://pypi.org/classifiers/ are allowed
    if lang_titlecase not in ('Irish', 'Lithuanian', 'Nepali'):
        classifiers.append('Natural Language :: ' + lang_titlecase)
    if lang in variants:
        long_desc += ' (Standard'
        for variant in variants[lang]:
            long_desc += ', ' + variant.title()
        long_desc += ')'
    long_desc += '\n'

long_desc += '''
This is a pure Python stemming library. If `PyStemmer <https://pypi.org/project/PyStemmer/>`_ is available, this module uses
it to accelerate.
'''

classifiers.extend([
    'Operating System :: OS Independent',
    'Programming Language :: Python',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 2.6',
    'Programming Language :: Python :: 2.7',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 3.4',
    'Programming Language :: Python :: 3.5',
    'Programming Language :: Python :: 3.6',
    'Programming Language :: Python :: 3.7',
    'Programming Language :: Python :: Implementation :: CPython',
    'Programming Language :: Python :: Implementation :: PyPy',
    'Topic :: Database',
    'Topic :: Internet :: WWW/HTTP :: Indexing/Search',
    'Topic :: Text Processing :: Indexing',
    'Topic :: Text Processing :: Linguistic'
])

setup(name='snowballstemmer',
      version='1.9.0',
      description=desc,
      long_description=long_desc,
      author='Snowball Developers',
      author_email='snowball-discuss@lists.tartarus.org',
      url='https://github.com/snowballstem/snowball',
      keywords="stemmer",
      license="BSD",
      packages=['snowballstemmer'],
      package_dir={"snowballstemmer": "src/snowballstemmer"},
      classifiers = classifiers
)
