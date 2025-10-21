import sys
import codecs
import snowballstemmer

def usage():
    print('''usage: %s [-l <language>] [-i <input file>] [-o <output file>] [-c <character encoding>] [-p[2]] [-h]

The input file consists of a list of words to be stemmed, one per
line. Words should be in lower case, but (for English) A-Z letters
are mapped to their a-z equivalents anyway. If omitted, stdin is
used.

If -c is given, the argument is the character encoding of the input
and output files.  If it is omitted, the UTF-8 encoding is used.

If -p is given the output file consists of each word of the input
file followed by \"->\" followed by its stemmed equivalent.
If -p2 is given the output file is a two column layout containing
the input words in the first column and the stemmed equivalents in
the second column.

Otherwise, the output file consists of the stemmed words, one per
line.

-h displays this help''' % sys.argv[0])

def main():
    pretty = 0
    input = ''
    output = ''
    encoding = 'utf_8'
    language = 'English'
    show_help = False
    argv = sys.argv[1:]
    while len(argv):
        arg = argv.pop(0)
        if arg == '-h':
            show_help = True
            break
        elif arg == "-p":
            pretty = 1
        elif arg == "-p2":
            pretty = 2
        elif arg == "-l":
            if len(argv) == 0:
                show_help = True
                break
            language = argv.pop(0)
        elif arg == "-i":
            if len(argv) == 0:
                show_help = True
                break
            input = argv.pop(0)
        elif arg == "-o":
            if len(argv) == 0:
                show_help = True
                break
            output = argv.pop(0)
        elif arg == "-c":
            if len(argv) == 0:
                show_help = True
                break
            encoding = argv.pop(0)
    if show_help:
        usage()
    else:
        stemmer = snowballstemmer.stemmer(language)
        if input != '':
            infile = codecs.open(input, "r", encoding)
        else:
            infile = sys.stdin
            # reconfigure() requires Python 3.7 so check existing encoding.
            if infile.encoding.lower() != encoding.lower():
                infile.reconfigure(encoding = encoding)
        if output != '':
                outfile = codecs.open(output, "w", encoding)
        else:
            outfile = sys.stdout
            if outfile.encoding.lower() != encoding.lower():
                outfile.reconfigure(encoding = encoding)
        stemming(stemmer, infile, outfile, pretty)
        outfile.close()
        infile.close()


def stemming(stemmer, infile, outfile, pretty):
    for original in infile.readlines():
        original = original.strip()
        # Convert only ASCII-letters to lowercase, to match C behavior
        original = ''.join(c.lower() if 'A' <= c <= 'Z' else c for c in original)
        stemmed = stemmer.stemWord(original)
        if pretty == 0:
            if stemmed != "":
                outfile.write(stemmed)
        elif pretty == 1:
            outfile.write(original, " -> ", stemmed)
        elif pretty == 2:
            outfile.write(original)
            if len(original) < 30:
                outfile.write(" " * (30 - len(original)))
            else:
                outfile.write("\n")
                outfile.write(" " * 30)
            outfile.write(stemmed)
        outfile.write('\n')

main()
