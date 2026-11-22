#!/usr/bin/env python3
"""Check that all classifiers in a built Python sdist are valid.

PyPI rejects uploads using classifiers which aren't listed in
https://pypi.org/classifiers/ - checking this in CI means we find out
before we try to deploy a release.

Usage: check_classifiers.py SDIST
"""

import sys
import tarfile
from email.parser import HeaderParser
from pathlib import Path

from trove_classifiers import classifiers as valid_classifiers


def main(args):
    if len(args) != 1:
        sys.exit(__doc__.strip())
    sdist = Path(args[0])
    # The sdist contains a single top-level directory with the same name.
    pkg_info = f"{sdist.name.removesuffix('.tar.gz')}/PKG-INFO"
    with tarfile.open(sdist) as tf:
        metadata = HeaderParser().parsestr(
            tf.extractfile(pkg_info).read().decode("utf-8")
        )
    if not (classifiers := metadata.get_all("Classifier")):
        sys.exit(f"{sdist}: No classifiers found")
    # Deprecated classifiers are not in valid_classifiers.
    if invalid := [c for c in classifiers if c not in valid_classifiers]:
        sys.exit(
            f"{sdist}: Invalid classifiers:\n"
            + "".join(f"  {c}\n" for c in invalid)
            + "Only classifiers listed in https://pypi.org/classifiers/ "
            "are allowed.  Remove them in python/setup.py or submit "
            "them to https://github.com/pypa/trove-classifiers"
        )
    print(f"{sdist}: {len(classifiers)} classifiers OK")


if __name__ == "__main__":
    main(sys.argv[1:])
