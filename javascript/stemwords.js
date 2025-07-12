import fs from 'node:fs';
import process from 'node:process';
import readline from 'node:readline';

function usage() {
    console.log(`usage: stemwords.js [-l <language>] [-i <input file>] [-o <output file>] [-c <character encoding>] [-h]

The input file consists of a list of words to be stemmed, one per line.
Words should be in lower case.

Language defaults to "English", input to stdin, and output to stdout.

If -c is given, the argument is the character encoding of the input and
output files.  If it is omitted, the UTF-8 encoding is used.

The output file consists of the stemmed words, one per line.

-h displays this help`);
}

if (process.argv.length < 5)
{
    usage();
}
else
{
    let input = '';
    let output = '';
    let encoding = 'utf8';
    let language = 'English';
    let show_help = false;
    // deno doesn't allow modifying process.argv so make a copy.
    const argv = process.argv.slice(0);
    while (argv.length > 0)
    {
        const arg = argv.shift();
        switch (arg)
        {
        case "-h":
            show_help = true;
            argv.length = 0;
            break;
        case "-l":
            if (argv.length === 0)
            {
                show_help = true;
                break;
            }
            language = argv.shift();
            break;
        case "-i":
            if (argv.length === 0)
            {
                show_help = true;
                break;
            }
            input = argv.shift();
            break;
        case "-o":
            if (argv.length === 0)
            {
                show_help = true;
                break;
            }
            output = argv.shift();
            break;
        case "-c":
            if (argv.length === 0)
            {
                show_help = true;
                break;
            }
            encoding = argv.shift();
            break;
        }
    }
    if (show_help)
    {
        usage();
    }
    else
    {
        const stemmer = await create(language);
        let istream, ostream;
        if (input !== '') {
           istream = fs.createReadStream(input, encoding);
        } else {
           istream = process.stdin;
           if (istream.setEncoding) istream.setEncoding(encoding);
        }
        if (output !== '') {
            ostream = fs.createWriteStream(output, encoding);
        } else {
            ostream = process.stdout;
            if (ostream.setEncoding) ostream.setEncoding(encoding);
        }

        stemming(stemmer, istream, ostream);
    }
}

// function stemming (stemmer : Stemmer, input : Stream, output : Stream) {
function stemming (stemmer, input, output) {
    const lines = readline.createInterface({
        input: input,
        terminal: false
    });
    lines.on('line', (original) => {
        output.write(stemmer.stemWord(original) + '\n');
    });
}

async function create (name) {
    const lc_name = name.toLowerCase();
    if (/\W/.test(lc_name) || lc_name === 'base') {
        console.log('Unknown stemming language: ' + name + '\n');
        usage();
        process.exit(1);
        return;
    }
    const stemmerName = `${titleCase(lc_name)}Stemmer`;
    const filename = `../js_out/${lc_name}-stemmer.js`;
    try {
        // Load stemmer class from the module scope
        const stemmerModule = await import(filename);
        return new stemmerModule[stemmerName]();
    } catch (error) {
        console.error(error);
    }
}

function titleCase (s) {
    return s.split('_').map(w => w.charAt(0).toUpperCase() + w.slice(1)).join('');
}
