const fs = require('fs');
const readline = require('readline');

function usage() {
    console.log("usage: stemwords.js [-l <language>] -i <input file> -o <output file> [-c <character encoding>] [--esm] [-h]\n");
    console.log("The input file consists of a list of words to be stemmed, one per");
    console.log("line. Words should be in lower case.\n");
    console.log("If -c is given, the argument is the character encoding of the input");
    console.log("and output files.  If it is omitted, the UTF-8 encoding is used.\n");
    console.log("The output file consists of the stemmed words, one per line.\n");
    console.log("Using --esm loads JavaScript Modules (ESM).\n");
    console.log("-h displays this help");
}

if (process.argv.length < 5)
{
    usage();
}
else
{
    var input = '';
    var output = '';
    var encoding = 'utf8';
    var language = 'English';
    var show_help = false;
    let use_esm = false;
    while (process.argv.length > 0)
    {
        var arg = process.argv.shift();
        switch (arg)
        {
        case "-h":
            show_help = true;
            process.argv.length = 0;
            break;
        case "-l":
            if (process.argv.length === 0)
            {
                show_help = true;
                break;
            }
            language = process.argv.shift();
            break;
        case "-i":
            if (process.argv.length === 0)
            {
                show_help = true;
                break;
            }
            input = process.argv.shift();
            break;
        case "-o":
            if (process.argv.length === 0)
            {
                show_help = true;
                break;
            }
            output = process.argv.shift();
            break;
        case "-c":
            if (process.argv.length === 0)
            {
                show_help = true;
                break;
            }
            encoding = process.argv.shift();
            break;
        case "--esm":
            use_esm = true;
            break;
        }
    }
    if (show_help || input === '' || output === '')
    {
        usage();
    }
    else
    {
        stemming(language, input, output, encoding, use_esm);
    }
}

// function stemming (lang : string, input : string, output : string, encoding : string, use_esm : Boolean) {
function stemming (lang, input, output, encoding, use_esm) {
    const lines = readline.createInterface({
        input: fs.createReadStream(input, encoding),
        terminal: false
    });
    var out = fs.createWriteStream(output, encoding);
    var stemmer = create(lang);
    lines.on('line', (original) => {
        out.write(stemmer.stemWord(original) + '\n');
    });
}

function create (name, use_esm) {
    let lc_name = name.toLowerCase();
    if (!lc_name.match('\\W') && lc_name !== 'base') {
        lc_name = `stemmer-${lc_name}`;
        const filename = use_esm ? `${lc_name}.esm.js` : `${lc_name}.js`;
        try {
            const Stemmer = require(filename);
            return new Stemmer();
        } catch (error) {
        }
    }
    console.log('Unknown stemming language: ' + name + '\n');
    usage();
    process.exit(1);
}
