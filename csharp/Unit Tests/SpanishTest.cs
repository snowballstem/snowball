using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class SpanishTest
    {
        // Some tests are based on NLTK test cases
        // https://raw.githubusercontent.com/nltk/nltk/develop/nltk/test/unit/test_stem.py

        [Test]
        public void Spanish_BaseTest()
        {
            var stemmer = new SpanishStemmer();

            Assert.AreEqual("acerqu", stemmer.Stem("acerquen"));
            Assert.AreEqual("Vision", stemmer.Stem("Visionado"));
            Assert.AreEqual("algu", stemmer.Stem("algue"));
        }

        [Test]
        public void Spanish_FullTest()
        {
            Tools.Test(new SpanishStemmer(), "spanish");
        }
    }
}
