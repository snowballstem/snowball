using NUnit.Framework;
using Snowball;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Unit_Tests
{
    public static class Tools
    {
        public static void Test(Stemmer stemmer, string language)
        {
            string snowballPath = Path.GetFullPath(Environment.CurrentDirectory);
            for (int i = 0; i < 5; i++)
                snowballPath = Directory.GetParent(snowballPath).FullName;

            string dataPath = Path.GetFullPath(Path.Combine(snowballPath, "snowball-data"));

            string langPath = Path.Combine(dataPath, language);

            string inputFile = Path.Combine(langPath, "voc.txt");
            string outputFile = Path.Combine(langPath, "output.txt");

            string input = File.ReadAllText(inputFile);
            string output = File.ReadAllText(outputFile);

            Test(stemmer, input, output);
        }

        public static void Test(Stemmer stemmer, string input, string output)
        {
            var newline = new[] { Environment.NewLine };
            var inputLines = input.Split(newline, StringSplitOptions.None);
            var outputLines = output.Split(newline, StringSplitOptions.None);

            for (int i = 0; i < inputLines.Length; i++)
            {
                string word = inputLines[i];
                string expected = outputLines[i];
                string actual = stemmer.Stem(word);

                Assert.AreEqual(expected, actual);
            }
        }
    }
}
