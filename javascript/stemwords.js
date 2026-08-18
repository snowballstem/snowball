import { Buffer } from 'node:buffer';
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

{
    let input;
    let output;
    /**@type {BufferEncoding}*/
    let encoding = 'utf8';
    let language = 'English';
    let usage_error = false;
    // Skip the first two entries of argv which are the interpreter
    // and the script name.
    //
    // deno doesn't allow modifying process.argv so we need to make
    // a copy here.
    const argv = process.argv.slice(2);
    while (argv.length > 0)
    {
        const arg = argv.shift();
        switch (arg)
        {
        case "-h":
            usage();
            process.exit(0);
            break;
        case "-l":
            if (argv.length === 0)
            {
                usage_error = true;
                break;
            }
            language = /**@type {string}*/ (argv.shift());
            break;
        case "-i":
            if (argv.length === 0)
            {
                usage_error = true;
                break;
            }
            input = argv.shift();
            break;
        case "-o":
            if (argv.length === 0)
            {
                usage_error = true;
                break;
            }
            output = argv.shift();
            break;
        case "-c":
            if (argv.length === 0)
            {
                usage_error = true;
                break;
            }
            {
                const enc = /**@type {string}*/ (argv.shift());
                if (!Buffer.isEncoding(enc))
                {
                    console.log('Unknown character encoding: ' + enc + '\n');
                    usage_error = true;
                    break;
                }
                encoding = enc;
            }
            break;
        default:
            console.log('Unknown command line option: ' + arg + '\n');
            usage_error = true;
        }

        if (usage_error)
        {
            usage();
            process.exit(1);
        }
    }

    const stemmer = await create(language);
    let istream, ostream;
    if (input !== undefined) {
       istream = fs.createReadStream(input, encoding);
    } else {
       istream = process.stdin;
       if (istream.setEncoding) istream.setEncoding(encoding);
    }
    if (output !== undefined) {
        ostream = fs.createWriteStream(output, encoding);
    } else {
        ostream = process.stdout;
        if (ostream.setEncoding) ostream.setEncoding(encoding);
    }

    stemming(stemmer, istream, ostream);
}

/**
 * @typedef {object} Stemmer
 * @property {function(string): string} stemWord
 */

/**
 * @param {Stemmer} stemmer
 * @param {NodeJS.ReadableStream} input
 * @param {NodeJS.WritableStream} output
 */
function stemming(stemmer, input, output) {
    const lines = readline.createInterface({
        input: input,
        terminal: false
    });
    lines.on('line', (original) => {
        output.write(stemmer.stemWord(original) + '\n');
    });
}

/**
 * @param {string} name
 * @return {Promise<Stemmer>} name
 */
async function create(name) {
    const lc_name = name.toLowerCase();
    if (/\W/.test(lc_name) || lc_name === 'base') {
        console.log('Unknown stemming language: ' + name + '\n');
        usage();
        process.exit(1);
    }
    const filename = `../js_out/${lc_name}-stemmer.js`;
    try {
        // Load stemmer class from the module scope
        const stemmerModule = await import(filename);
        return new stemmerModule.default();
    } catch (error) {
        console.error(error);
        process.exit(1);
    }
}
