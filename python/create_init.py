#! /bin/sh/env python

import sys
import re
import os
from typing import Dict, List, Type

python_out_folder = sys.argv[1]

filematch = re.compile(r"(\w+)_stemmer\.py$")

imports = []
languages = []

for pyscript in os.listdir(python_out_folder):
    match = filematch.match(pyscript)
    if (match):
        langname = match.group(1)
        titlecase = re.sub(r"_", "", langname.title())
        languages.append("        '%(lang)s': %(title)sStemmer," % {'lang': langname, 'title': titlecase})
        imports.append('    from .%(lang)s_stemmer import %(title)sStemmer' % {'lang': langname, 'title': titlecase})
imports.sort()
languages.sort()

if len(languages) == 0:
    raise AssertionError('languages list is empty!')

src = '''from typing import Dict, List, Type

__all__ = ('algorithms', 'stemmer')

try:
    import Stemmer
    algorithms = Stemmer.algorithms
    stemmer = Stemmer.Stemmer
except ImportError:
    from .basestemmer import BaseStemmer
%(imports)s

    _languages: Dict[str, Type[BaseStemmer]] = {
%(languages)s
    }

    def algorithms() -> List[str]:
        return list(_languages.keys())

    def stemmer(lang: str) -> BaseStemmer:
        lang = lang.lower()
        if lang in _languages:
            return _languages[lang]()
        else:
            raise KeyError("Stemming algorithm '%%s' not found" %% lang)
''' % {'imports': '\n'.join(imports), 'languages': '\n'.join(languages)}

with open(os.path.join(python_out_folder, '__init__.py'), 'w') as out:
    out.write(src)
