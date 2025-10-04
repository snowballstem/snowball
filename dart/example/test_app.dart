import 'dart:convert';
import 'dart:io';

import 'package:snowball/snowball.dart';

void usage() {
  print("Usage: test_app <algorithm> [<input file>] [-o <output file>]");
}

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    usage();
    exit(1);
  }

  final algorithms = Algorithm.values.where((v) => v.name == args[0]);
  if (algorithms.isEmpty) {
    print('Stemmer for ${args[0]} not found');
    exit(1);
  }
  final algorithm = algorithms.first;
  final stemmer = SnowballStemmer(algorithm);

  int arg = 1;

  Stream<List<int>> inStream;
  if (args.length > arg && args[arg] != '-o') {
    inStream = File(args[arg++]).openRead();
  } else {
    inStream = stdin;
  }

  IOSink outStream;
  if (args.length > arg) {
    if (args.length != arg + 2 || args[arg] != '-o') {
      usage();
      exit(1);
    }
    outStream = File(args[arg + 1]).openWrite();
  } else {
    outStream = stdout.nonBlocking;
  }

  Stream<String> reader = inStream
      .transform(utf8.decoder)
      .transform(LineSplitter());

  final outBuffer = StringBuffer();
  await for (var line in reader) {
    try {
      final stem = stemmer.stem(line);
      outBuffer.writeln(stem);
      if (outBuffer.length > 8192) {
        outStream.write(outBuffer);
        outBuffer.clear();
      }
    } catch (e) {
      print('Failed to stem word "$line"');
      rethrow;
    }
  }
  if (outBuffer.isNotEmpty) outStream.write(outBuffer);
  await outStream.flush();
  await outStream.close();
}
