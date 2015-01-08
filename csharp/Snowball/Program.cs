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
            Console.WriteLine("Usage: Steamer.exe <algorithm> <input file> [-o <output file>]");
        }

        public static void Main(String[] args)
        {
            if (args.Length < 2)
            {
                usage();
                return;
            }

            string stemmerName = args[0];

            SnowballStemmer stemmer =
                typeof(SnowballStemmer).Assembly.GetTypes()
                    .Where(t => t.IsSubclassOf(typeof(SnowballStemmer)) && !t.IsAbstract)
                    .Where(t => t.Name.Contains(stemmerName))
                    .Select(t => (SnowballStemmer)Activator.CreateInstance(t)).FirstOrDefault();

            TextReader reader = new StreamReader(args[1]);
            StringBuilder input = new StringBuilder();
            TextWriter output;

            if (args.Length > 2)
            {
                if (args.Length >= 4 && args[2] == "-o")
                {
                    output = new StreamWriter(args[3]);
                }
                else
                {
                    usage();
                    return;
                }
            }
            else
            {
                output = System.Console.Out;
            }


            int repeat = 1;
            if (args.Length > 4)
            {
                repeat = int.Parse(args[4]);
            }


            int character;
            while ((character = reader.Read()) != -1)
            {
                char ch = (char)character;
                if (char.IsWhiteSpace((char)ch))
                {
                    if (input.Length > 0)
                    {
                        stemmer.setCurrent(input.ToString());
                        for (int i = repeat; i != 0; i--)
                        {
                            stemmer.Stem();
                        }

                        output.WriteLine(stemmer.getCurrent());
                        input.Remove(0, input.Length);
                    }
                }
                else
                {
                    input.Append(char.ToLowerInvariant(ch));
                }
            }

            output.Flush();
        }
    }
}