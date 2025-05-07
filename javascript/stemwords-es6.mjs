import fs from 'fs';
import readline from 'readline';

const usage = () => {
    console.log(`usage: stemwords-es6.mjs [-l <language>] -i <input file> -o <output file> [-c <character encoding>] [-h]

The input file consists of a list of words to be stemmed, one per
line. Words should be in lower case.

If -c is given, the argument is the character encoding of the input
and output files.  If it is omitted, the UTF-8 encoding is used.

The output file consists of the stemmed words, one per line.

-h displays this help`);
};

const stemming = async (lang, input, output, encoding) => {
    const stemmer = await create(lang);

    const inputStream = fs.createReadStream(input, { encoding });
    const lines = readline.createInterface({
        input: inputStream,
        terminal: false
    });

    const out = fs.createWriteStream(output, { encoding });

    return new Promise((resolve, reject) => {
        lines.on('line', (original) => {
            const stemmed = stemmer.stemWord(original);
            out.write(`${stemmed}\n`);
        });

        lines.on('close', () => {
            out.end();
            resolve();
        });

        lines.on('error', (err) => {
            reject(err);
        });

        out.on('error', (err) => {
            reject(err);
        });
    });
};

const create = async (name) => {
    const lc_name = name.toLowerCase();
    if (!lc_name.match('\\W') && lc_name !== 'base') {
        try {
            const Stemmer = await import(`./${lc_name}-stemmer-es6.mjs`);
            return Stemmer.default;
        } catch (error) {
            console.error(`Error loading stemmer for language ${name}: ${error}\n`);
            process.exit(1);
        }
    }
    console.error(`Unknown stemming language: ${name}\n`);
    usage();
    process.exit(1);
};

const main = async () => {
    let input = '';
    let output = '';
    let encoding = 'utf8';
    let language = 'English';
    let show_help = false;

    const args = [...process.argv.slice(2)]; // Copy the arguments for processing

    while (args.length > 0) {
        const arg = args.shift();
        switch (arg) {
            case "-h":
                show_help = true;
                break;
            case "-l":
                if (args.length === 0) {
                    show_help = true;
                    break;
                }
                language = args.shift();
                break;
            case "-i":
                if (args.length === 0) {
                    show_help = true;
                    break;
                }
                input = args.shift();
                break;
            case "-o":
                if (args.length === 0) {
                    show_help = true;
                    break;
                }
                output = args.shift();
                break;
            case "-c":
                if (args.length === 0) {
                    show_help = true;
                    break;
                }
                encoding = args.shift();
                break;
            default:
                console.error(`Unknown option: ${arg}`);
                show_help = true;
        }
        if (show_help) break;
    }

    if (show_help || input === '' || output === '') {
        usage();
        process.exit(show_help ? 0 : 1);
    } else {
        try {
            await stemming(language, input, output, encoding);
        } catch (error) {
            console.error('An error occurred:', error);
            process.exit(1);
        }
    }
}

main();
