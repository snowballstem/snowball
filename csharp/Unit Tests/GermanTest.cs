using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class GermanTest
    {
        // Some tests are based on NLTK test cases
        // https://raw.githubusercontent.com/nltk/nltk/develop/nltk/test/unit/test_stem.py

        [Test]
        public void German_BaseTest()
        {
            GermanStemmer german = new GermanStemmer();

            Assert.AreEqual("Schrank", german.Stem("Schr\xe4nke"));
            Assert.AreEqual("kein", german.Stem("keinen"));
            Assert.AreEqual("aeternitatis", german.Stem("aeternitatis"));
            Assert.AreEqual("affar", german.Stem("affäre"));
        }

        [Test]
        public void German_PreludeTest()
        {
            GermanStemmer german = new GermanStemmer();

            Assert.AreEqual("ablasst", german.Stem("abläßt"));
        }

        [Test]
        public void German_FullTest()
        {
            Tools.Test(new GermanStemmer(), "german");
        }

    }
}
