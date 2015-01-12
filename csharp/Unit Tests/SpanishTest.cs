using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class SpanishTest
    {
        // Some tests are based on NLTK test cases
        // https://raw.githubusercontent.com/nltk/nltk/develop/nltk/test/unit/test_stem.py

        [TestMethod]
        public void Spanish_BaseTest()
        {
            var stemmer = new SpanishStemmer();

            Assert.AreEqual("Vision", stemmer.Stem("Visionado"));
            Assert.AreEqual("algu", stemmer.Stem("algue"));
        }

        [TestMethod]
        public void Spanish_FullTest()
        {
            Tools.Test(new SpanishStemmer(), "spanish");
        }
    }
}
