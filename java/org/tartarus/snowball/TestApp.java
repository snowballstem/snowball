
package org.tartarus.snowball;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.nio.charset.StandardCharsets;

public class TestApp {
    private static void usage()
    {
        System.err.println("Usage: TestApp <algorithm> [<input file>] [-o <output file>]");
    }

    private static SnowballStemmer getStemmer(String lang) {
        try {
            String c = "org.tartarus.snowball.ext." + lang + "Stemmer";
            return (SnowballStemmer) Class.forName(c).getDeclaredConstructor().newInstance();
        } catch (ReflectiveOperationException e) {
            return null;
        }
    }

    public static void main(String [] args) throws Throwable {
        if (args.length < 1) {
            usage();
            return;
        }

        SnowballStemmer stemmer = getStemmer(args[0]);
        if (stemmer == null) {
            System.err.println("Stemmer " + args[0] + " not found");
            return;
        }

	int arg = 1;

	InputStream instream;
	if (args.length > arg && !args[arg].equals("-o")) {
	    instream = new FileInputStream(args[arg++]);
	} else {
	    instream = System.in;
	}

        OutputStream outstream;
	if (args.length > arg) {
            if (args.length != arg + 2 || !args[arg].equals("-o")) {
                usage();
                return;
            }
	    outstream = new FileOutputStream(args[arg + 1]);
	} else {
	    outstream = System.out;
	}

	Reader reader = new InputStreamReader(instream, StandardCharsets.UTF_8);
	reader = new BufferedReader(reader);

	Writer output = new OutputStreamWriter(outstream, StandardCharsets.UTF_8);
	output = new BufferedWriter(output);

	StringBuffer input = new StringBuffer();
	int character;
	while ((character = reader.read()) != -1) {
	    char ch = (char) character;
	    if (Character.isWhitespace(ch)) {
		stemmer.setCurrent(input.toString());
		stemmer.stem();
		output.write(stemmer.getCurrent());
		output.write('\n');
		input.delete(0, input.length());
	    } else {
		input.append(ch < 127 ? Character.toLowerCase(ch) : ch);
	    }
	}
	output.flush();
    }
}
