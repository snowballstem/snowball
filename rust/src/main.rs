use std::fs::File;
use std::io::{BufRead, BufReader, Write};
use std::path::Path;
use std::env;
use std::borrow::Cow;

pub mod snowball;

use snowball::SnowballEnv;


fn usage(name: &str) {
    println!("{} -l <language> [-i <input file>] [-o <output file>]
The input file consists of a list of words to be stemmed, one per
line. Words should be in lower case, but (for English) A-Z letters
are mapped to their a-z equivalents anyway. If omitted, stdin is
used.", name);
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 3 {
        usage(&args[0]);
    } else {
        let mut language = None;
        let mut input_arg = None;
        let mut output_arg = None;
        let mut i = 1;
        while i < args.len() {
            match args[i].as_str() {
                "-l" => {
                    language = Some(args[i+1].clone());
                    i += 2;
                },
                "-i" => {
                    input_arg = Some(args[i+1].clone());
                    i += 2;
                },
                "-o" => {
                    output_arg = Some(args[i+1].clone());
                    i += 2;
                },
                x => {
                    println!("Unrecognized option '{}'", x);
                    usage(&args[0]);
                    return
                }
            }
        }
        if language.is_none() {
            println!("Please specify a language!");
            usage(&args[0]);
            return;
        }
        let stemmer = Stemmer::create(language.unwrap());
        

        let mut output = if let Some(output_file) = output_arg {
            Box::new(File::create(Path::new(&output_file)).unwrap()) as Box<Write>
        } else {
            Box::new(std::io::stdout()) as Box<Write>
        };

        if let Some(input_file) = input_arg {
            for line in BufReader::new(File::open(Path::new(&input_file)).unwrap()).lines() {
                writeln!(&mut output, "{}", stemmer.stem(&line.unwrap())).unwrap();
            }
        } else {
            let stdin = std::io::stdin();
            for line in stdin.lock().lines() {
                writeln!(&mut output, "{}", stemmer.stem(&line.unwrap())).unwrap();
            }
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
        match lang.as_str() {
            "arabic" => Stemmer { stemmer: Box::new(algorithms::arabic_stemmer::stem) },
            "danish" => Stemmer { stemmer: Box::new(algorithms::danish_stemmer::stem) },
            "dutch" => Stemmer { stemmer: Box::new(algorithms::dutch_stemmer::stem) },
            "english" => Stemmer { stemmer: Box::new(algorithms::english_stemmer::stem) },
            "finnish" => Stemmer { stemmer: Box::new(algorithms::finnish_stemmer::stem) },
            "french" => Stemmer { stemmer: Box::new(algorithms::french_stemmer::stem) },
            "german" => Stemmer { stemmer: Box::new(algorithms::german_stemmer::stem) },
            "hungarian" => Stemmer { stemmer: Box::new(algorithms::hungarian_stemmer::stem) },
            "italian" => Stemmer { stemmer: Box::new(algorithms::italian_stemmer::stem) },
            "norwegian" => Stemmer { stemmer: Box::new(algorithms::norwegian_stemmer::stem) },
            "porter" => Stemmer { stemmer: Box::new(algorithms::porter_stemmer::stem) },
            "portuguese" => Stemmer { stemmer: Box::new(algorithms::portuguese_stemmer::stem) },
            "romanian" => Stemmer { stemmer: Box::new(algorithms::romanian_stemmer::stem) },
            "russian" => Stemmer { stemmer: Box::new(algorithms::russian_stemmer::stem) },
            "spanish" => Stemmer { stemmer: Box::new(algorithms::spanish_stemmer::stem) },
            "swedish" => Stemmer { stemmer: Box::new(algorithms::swedish_stemmer::stem) },
            "tamil" => Stemmer { stemmer: Box::new(algorithms::tamil_stemmer::stem) },
            "turkish" => Stemmer { stemmer: Box::new(algorithms::turkish_stemmer::stem) },
            x => panic!("Unknown algorithm '{}'", x)
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
