using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class GermanTest
    {
        // Some tests are based on NLTK test cases
        // https://raw.githubusercontent.com/nltk/nltk/develop/nltk/test/unit/test_stem.py

        [TestMethod]
        public void German_BaseTest()
        {
            GermanStemmer german = new GermanStemmer();

            Assert.AreEqual("Schrank", german.Stem("Schr\xe4nke"));
            Assert.AreEqual("kein", german.Stem("keinen"));
        }

    }
}
