// ignore_for_file: non_constant_identifier_names, curly_braces_in_flow_control_structures

library;

import 'src/algorithms.dart';
export 'src/algorithms.dart';
import 'src/snowball.dart' as impl;

class SnowballStemmer {
  final impl.SnowballStemmer _stemmer;

  SnowballStemmer(Algorithm algorithm) : _stemmer = stemmers[algorithm]!;
  String stem(String s) {
    _stemmer.init(s);
    _stemmer.stem();
    return _stemmer.current;
  }
}
