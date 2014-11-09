#!/bin/bash

MG4J_PKG_DIR="java_law/it/unimi/di/big/mg4j/index/snowball"

rm $MG4J_PKG_DIR/*Stemmer.*

ALGO_DIR="algorithms"
ISO_8859_ALGO="stem_ISO_8859_1.sbl"
SNOWBALL_COMPILER="./snowball"

for algo in `ls $ALGO_DIR`; do
	if [ -f $ALGO_DIR/$algo/$ISO_8859_ALGO ]; then
		first_char=`echo "${algo:0:1}" | tr a-z A-Z`
		rest=${algo:1}
		rest=${rest/_p/P}
		CLASS=$first_char$rest"Stemmer"
		echo $SNOWBALL_COMPILER $ALGO_DIR/$algo/$ISO_8859_ALGO $GEN_JAVA_CODE -o $MG4J_PKG_DIR"/"$CLASS -n $CLASS
		$SNOWBALL_COMPILER $ALGO_DIR/$algo/$ISO_8859_ALGO -java -o $MG4J_PKG_DIR"/"$CLASS \
				-n $CLASS \
				-p it.unimi.di.big.mg4j.index.snowball.AbstractSnowballTermProcessor \
				-a it.unimi.di.big.mg4j.index.snowball.Among \
				-P it.unimi.di.big.mg4j.index.snowball \
				-S it.unimi.dsi.lang.MutableString \
				-B it.unimi.di.big.mg4j.index.snowball.BooleanStrategy
	fi
done
