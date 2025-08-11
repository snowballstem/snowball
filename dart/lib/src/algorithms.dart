// ignore_for_file: constant_identifier_names

import 'package:snowball/src/snowball.dart';

import '../ext/arabic_stemmer.dart';
import '../ext/armenian_stemmer.dart';
import '../ext/basque_stemmer.dart';
import '../ext/catalan_stemmer.dart';
import '../ext/danish_stemmer.dart';
import '../ext/dutch_stemmer.dart';
import '../ext/dutch_porter_stemmer.dart';
import '../ext/english_stemmer.dart';
import '../ext/esperanto_stemmer.dart';
import '../ext/estonian_stemmer.dart';
import '../ext/finnish_stemmer.dart';
import '../ext/french_stemmer.dart';
import '../ext/german_stemmer.dart';
import '../ext/greek_stemmer.dart';
import '../ext/hindi_stemmer.dart';
import '../ext/hungarian_stemmer.dart';
import '../ext/indonesian_stemmer.dart';
import '../ext/irish_stemmer.dart';
import '../ext/italian_stemmer.dart';
import '../ext/lithuanian_stemmer.dart';
import '../ext/nepali_stemmer.dart';
import '../ext/norwegian_stemmer.dart';
import '../ext/porter_stemmer.dart';
import '../ext/portuguese_stemmer.dart';
import '../ext/romanian_stemmer.dart';
import '../ext/russian_stemmer.dart';
import '../ext/serbian_stemmer.dart';
import '../ext/spanish_stemmer.dart';
import '../ext/swedish_stemmer.dart';
import '../ext/tamil_stemmer.dart';
import '../ext/turkish_stemmer.dart';
import '../ext/yiddish_stemmer.dart';

enum Algorithm {
  arabic,
  armenian,
  basque,
  catalan,
  danish,
  dutch,
  dutch_porter,
  english,
  esperanto,
  estonian,
  finnish,
  french,
  german,
  greek,
  hindi,
  hungarian,
  indonesian,
  irish,
  italian,
  lithuanian,
  nepali,
  norwegian,
  porter,
  portuguese,
  romanian,
  russian,
  serbian,
  spanish,
  swedish,
  tamil,
  turkish,
  yiddish,
}

final stemmers = <Algorithm, SnowballStemmer>{
  Algorithm.arabic: arabic_stemmer(),
  Algorithm.armenian: armenian_stemmer(),
  Algorithm.basque: basque_stemmer(),
  Algorithm.catalan: catalan_stemmer(),
  Algorithm.danish: danish_stemmer(),
  Algorithm.dutch: dutch_stemmer(),
  Algorithm.dutch_porter: dutch_porter_stemmer(),
  Algorithm.english: english_stemmer(),
  Algorithm.esperanto: esperanto_stemmer(),
  Algorithm.estonian: estonian_stemmer(),
  Algorithm.finnish: finnish_stemmer(),
  Algorithm.french: french_stemmer(),
  Algorithm.german: german_stemmer(),
  Algorithm.greek: greek_stemmer(),
  Algorithm.hindi: hindi_stemmer(),
  Algorithm.hungarian: hungarian_stemmer(),
  Algorithm.indonesian: indonesian_stemmer(),
  Algorithm.irish: irish_stemmer(),
  Algorithm.italian: italian_stemmer(),
  Algorithm.lithuanian: lithuanian_stemmer(),
  Algorithm.nepali: nepali_stemmer(),
  Algorithm.norwegian: norwegian_stemmer(),
  Algorithm.porter: porter_stemmer(),
  Algorithm.portuguese: portuguese_stemmer(),
  Algorithm.romanian: romanian_stemmer(),
  Algorithm.russian: russian_stemmer(),
  Algorithm.serbian: serbian_stemmer(),
  Algorithm.spanish: spanish_stemmer(),
  Algorithm.swedish: swedish_stemmer(),
  Algorithm.tamil: tamil_stemmer(),
  Algorithm.turkish: turkish_stemmer(),
  Algorithm.yiddish: yiddish_stemmer(),
};
