const stemmer = require('base-stemmer.js');

const arabic_stemmer = require('arabic-stemmer.js');
const danish_stemmer = require('danish-stemmer.js');
const dutch_stemmer = require('dutch-stemmer.js');
const english_stemmer = require('english-stemmer.js');
const finnish_stemmer = require('finnish-stemmer.js');
const french_stemmer = require('french-stemmer.js');
const german_stemmer = require('german-stemmer.js');
const hungarian_stemmer = require('hungarian-stemmer.js');
const irish_stemmer = require('irish-stemmer.js');
const italian_stemmer = require('italian-stemmer.js');
const norwegian_stemmer = require('norwegian-stemmer.js');
const porter_stemmer = require('porter-stemmer.js');
const portuguese_stemmer = require('portuguese-stemmer.js');
const romanian_stemmer = require('romanian-stemmer.js');
const russian_stemmer = require('russian-stemmer.js');
const spanish_stemmer = require('spanish-stemmer.js');
const swedish_stemmer = require('swedish-stemmer.js');
const tamil_stemmer = require('tamil-stemmer.js');
const turkish_stemmer = require('turkish-stemmer.js');

const fs = require('fs');

function usage() {
    console.log("usage: jsx_stemwords [-l <language>] [-i <input file>] [-o <output file>] [-c <character encoding>] [-p[2]] [-h]\n");
    console.log("The input file consists of a list of words to be stemmed, one per");
    console.log("line. Words should be in lower case, but (for English) A-Z letters");
    console.log("are mapped to their a-z equivalents anyway. If omitted, stdin is");
    console.log("used.\n");
    console.log("If -c is given, the argument is the character encoding of the input");
    console.log("and output files.  If it is omitted, the UTF-8 encoding is used.\n");
    console.log("If -p is given the output file consists of each word of the input");
    console.log("file followed by \"->\" followed by its stemmed equivalent.");
    console.log("If -p2 is given the output file is a two column layout containing");
    console.log( "the input words in the first column and the stemmed eqivalents in");
    console.log("the second column.\n");
    console.log("Otherwise, the output file consists of the stemmed words, one per");
    console.log("line.\n");
    console.log("-h displays this help");
}

if (process.argv.length < 5)
{
    usage();
}
else
{
    var pretty = 0;
    var input = '';
    var output = '';
    var encoding = 'utf8';
    var language = 'English';
    var show_help = false;
    while (process.argv.length > 0)
    {
	var arg = process.argv.shift();
	switch (arg)
	{
	case "-h":
	    show_help = true;
	    process.argv.length = 0;
	    break;
	case "-p":
	    pretty = 1;
	    break;
	case "-p2":
	    pretty = 2;
	    break;
	case "-l":
	    if (process.argv.length == 0)
	    {
		show_help = true;
		break;
	    }
	    language = process.argv.shift();
	    break;
	case "-i":
	    if (process.argv.length == 0)
	    {
		show_help = true;
		break;
	    }
	    input = process.argv.shift();
	    break;
	case "-o":
	    if (process.argv.length == 0)
	    {
		show_help = true;
		break;
	    }
	    output = process.argv.shift();
	    break;
	case "-c":
	    if (process.argv.length == 0)
	    {
		show_help = true;
		break;
	    }
	    encoding = process.argv.shift();
	    break;
	}
    }
    if (show_help || input == '' || output == '')
    {
	usage();
    }
    else
    {
	stemming(language, input, output, encoding, pretty);
    }
}

// function stemming (lang : string, input : string, output : string, encoding : string, pretty : int) {
function stemming (lang, input, output, encoding, pretty) {
        var lines = fs.readFileSync(input, encoding).split("\n");
        var stemmer = create(lang);
        for (var i in lines)
        {
            var original = lines[i];
            var stemmed = stemmer.stemWord(original);
            switch (pretty)
            {
            case 0:
                lines[i] = stemmed;
                break;
            case 1:
                lines[i] += " -> " + stemmed;
                break;
            case 2:
                if (original.length < 30)
                {
                    for (var j = original.length; j < 30; j++)
                    {
                        lines[i] += " ";
                    }
                }
                else
                {
                    lines[i] += "\n";
                    for (var j = 0; j < 30; j++)
                    {
                        lines[i] += " ";
                    }
                }
                lines[i] += stemmed;
            }
        }
        fs.writeFileSync(output, lines.join('\n'), encoding);
}

function create (name) {
    var algo = name.substr(0, 1).toUpperCase() + name.substr(1).toLowerCase();
    if (algo != 'Base') {
	try {
	    return Function('"use strict";return new ' + algo + 'Stemmer()')();
	} catch (error) {
	    console.log(error)
	}
    }
    usage();
    process.exit(1);
}
