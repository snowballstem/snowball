#!/usr/bin/env bash
###################

LOCAL=$(pwd)

ALGORITHMS=../algorithms/
SNOWBALL=$LOCAL/../snowball
TARGET=$LOCAL/Snowball/Algorithms

function Compile {

  Language=$1
  RelativePath=$2
  CompletePath=${ALGORITHMS}$RelativePath
  ClassName=${Language}Stemmer
  OutputPath=${TARGET}/${ClassName}.generated

  echo "  - Processing $Language ($RelativePath)"
  $SNOWBALL $CompletePath -cs -o $OutputPath -name $ClassName
}


echo ""
echo " Stemmer generation script   "
echo ""
echo "  - Source:    $ALGORITHMS   "
echo "  - Target:    $TARGET       "
echo "  - Compiler:  $SNOWBALL     "
echo "                             "

# pause

echo

if [ -d "$TARGET" ]; then
    echo "  Clearning target directory"
    cd $TARGET
    rm -f *.cs
    cd $LOCAL
fi

echo "  Starting code generation"
echo ""


Compile "Danish"         "danish/stem_ISO_8859_1.sbl"             
Compile "Dutch"          "dutch/stem_ISO_8859_1.sbl"
Compile "English"        "english/stem_ISO_8859_1.sbl"
Compile "Finnish"        "finnish/stem_ISO_8859_1.sbl"
Compile "French"         "french/stem_ISO_8859_1.sbl"
Compile "German"         "german/stem_ISO_8859_1.sbl"
Compile "German2"        "german2/stem_ISO_8859_1.sbl"
Compile "Hungarian"      "hungarian/stem_Unicode.sbl"     
Compile "Italian"        "italian/stem_ISO_8859_1.sbl"
Compile "KraaijPohlmann" "kraaij_pohlmann/stem_ISO_8859_1.sbl"
Compile "Lovins"         "lovins/stem_ISO_8859_1.sbl"
Compile "Norwegian"      "norwegian/stem_ISO_8859_1.sbl"
Compile "Porter"         "porter/stem_ISO_8859_1.sbl"
Compile "Portuguese"     "portuguese/stem_ISO_8859_1.sbl"
Compile "Romanian"       "romanian/stem_Unicode.sbl"
Compile "Russian"        "russian/stem_Unicode.sbl"
Compile "Spanish"        "spanish/stem_ISO_8859_1.sbl"
Compile "Swedish"        "swedish/stem_ISO_8859_1.sbl"
Compile "Turkish"        "turkish/stem_Unicode.sbl"

echo
# pause

exit
