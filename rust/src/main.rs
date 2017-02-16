use std::fs::File;
use std::io::{Read, BufRead, Write};
use std::path::Path;
use std::env;

pub mod snowball;


fn usage(name: String) {
    println!("{} -l <language> [-i <input file>] [-o <output file>]
The input file consists of a list of words to be stemmed, one per
line. Words should be in lower case, but (for English) A-Z letters
are mapped to their a-z equivalents anyway. If omitted, stdin is
used."; name);
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args < 7 {
        usage(args[0]);
    } else {
        let language = None;
        let input_arg = None;
        let output_arg = None;
        for i in 1..args.len() {
            match args[i] {
                "-l" => {
                    language = Some(args[i+1].clone());
                    i += 2;
                },
                "-i" => {
                    input = Some(args[i+1].clone());
                    i += 2;
                },
                "-o" => {
                    output = Some(args[i+1].clone());
                    i += 2;
                },
                x => {
                    println!("Unrecognized option '{}'", x);
                    usage(args[0]);
                    return
                }
            }
        }
        let stemmer = Stemmer::create(language);
        let mut input = if let Some(input_file) = input_arg {
            File::open(Path::new(input_file)).unwrap()
        } else {
            std::io::stdin().lock()
        }

        let mut output = if let Some(output_file) = output_arg {
            File::open(Path::new(output_file)).unwrap()
        } else {
            std::io::stdout()
        }

        for line in input.lines() {
            writeln!(&mut output, "{}", stemmer.stem(line));
        }
    }
}


/// Wrapps a usable interface around the actual stemmer implementation
pub struct Stemmer {
    stemmer: Box<Fn(&mut SnowballEnv) -> bool>,
}

impl Stemmer {
    /// Create a new stemmer from an algorithm
    pub fn create(lang: String) -> Self {
        use snowball::algorithms;
        match lang {
            "arabic" => Stemmer { stemmer: Box::new(algorithms::arabic_stemmer::stem) },
            "english" => Stemmer { stemmer: Box::new(algorithms::english_stemmer::stem) },
            "finish" => Stemmer { stemmer: Box::new(algorithms::finnish_stemmer::stem) },
            "french" => Stemmer { stemmer: Box::new(algorithms::french_stemmer::stem) },
            "german" => Stemmer { stemmer: Box::new(algorithms::german_stemmer::stem) },
            "italian" => Stemmer { stemmer: Box::new(algorithms::italian_stemmer::stem) },
            "portuguese" => Stemmer { stemmer: Box::new(algorithms::portuguese_stemmer::stem) },
            "romanian" => Stemmer { stemmer: Box::new(algorithms::romanian_stemmer::stem) },
            "russian" => Stemmer { stemmer: Box::new(algorithms::russian_stemmer::stem) },
            "spanish" => Stemmer { stemmer: Box::new(algorithms::spanish_stemmer::stem) },
            x => panic!("Unkown algorithm ''", x)
        }
    }

    /// Stem a single word
    /// Please note, that the input is expected to be all lowercase (if that is applicable).
    pub fn stem<'a>(&self, input: &'a str) -> Cow<'a, str> {
        let mut env = SnowballEnv::create(input);
        (self.stemmer)(&mut env);
        env.get_current()
    }
}
