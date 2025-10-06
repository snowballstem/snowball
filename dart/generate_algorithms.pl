#!/usr/bin/env perl
use strict;
use warnings;

my @algorithms = @ARGV;

# Generate the lib/src/algorithms.dart file from modules.txt
print("// ignore_for_file: constant_identifier_names\n\n");
print("import 'package:snowball/src/snowball.dart';\n\n");
foreach my $algorithm (@algorithms) {
    print("import '../ext/${algorithm}_stemmer.dart';\n");
}
print("\nenum Algorithm {\n");
foreach my $algorithm (@algorithms) {
    print("  $algorithm,\n");
}
print("}\n\nfinal stemmers = <Algorithm, SnowballStemmer>{\n");
foreach my $algorithm (@algorithms) {
    print("  Algorithm.$algorithm: ${algorithm}_stemmer(),\n");
}
print("};\n");
