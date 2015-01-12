using System;
using System.IO;
using System.Reflection;
using System.Linq;
using System.Text;

namespace Snowball
{

    public static class Program
    {

        private static void usage()
        {
            Console.WriteLine("Usage: stemwords.exe -l <language> -i <input file> [-o <output file>]");
        }

        public static void Main(String[] args)
        {
            string language = null;
            string inputName = null;
            string outputName = null;

            for (int i = 0; i < args.Length; i++)
            {
                if (args[i] == "-l")
                    language = args[i + 1];
                else if (args[i] == "-i")
                    inputName = args[i + 1];
                if (args[i] == "-o")
                    outputName = args[i + 1];
            }

            if (language == null || inputName == null)
            {
                usage();
                return;
            }



            SnowballStemmer stemmer =
                typeof(SnowballStemmer).Assembly.GetTypes()
                    .Where(t => t.IsSubclassOf(typeof(SnowballStemmer)) && !t.IsAbstract)
                    .Where(t => match(t.Name, language))
                    .Select(t => (SnowballStemmer)Activator.CreateInstance(t)).FirstOrDefault();

            if (stemmer == null)
            {
                Console.WriteLine("Language not found.");
                return;
            }

            Console.WriteLine("Using " + stemmer.GetType());

            StringBuilder input = new StringBuilder();
            TextWriter output = System.Console.Out;

            if (outputName != null)
                output = new StreamWriter(outputName);


            foreach (var line in File.ReadAllLines(inputName))
            {
                var o = stemmer.Stem(line);
                output.WriteLine(o);
            }

            output.Flush();
        }

        private static bool match(string stemmerName, string language)
        {
            string expectedName = language.Replace("_", "") + "Stemmer";

            return stemmerName.StartsWith(expectedName,
                StringComparison.CurrentCultureIgnoreCase);
        }
    }
}