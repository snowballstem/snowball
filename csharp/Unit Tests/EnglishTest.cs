﻿using System;
using NUnit.Framework;
using Snowball;
using Unit_Tests.Properties;

namespace Unit_Tests
{
    [TestFixture]
    public class EnglishTest
    {
        // Some tests are based on NLTK test cases
        // https://raw.githubusercontent.com/nltk/nltk/develop/nltk/test/unit/test_stem.py

        [Test]
        public void English_BaseTest()
        {
            EnglishStemmer stemmer = new EnglishStemmer();

            Assert.AreEqual("do", stemmer.Stem("doing"));
            Assert.AreEqual("andes", stemmer.Stem("andes"));
            Assert.AreEqual("coincidenti", stemmer.Stem("coincidential"));
            Assert.AreEqual("ration", stemmer.Stem("rationalism"));

            Assert.AreEqual("caress", stemmer.Stem("caresses"));
            Assert.AreEqual("fli", stemmer.Stem("flies"));
            Assert.AreEqual("die", stemmer.Stem("dies"));
            Assert.AreEqual("mule", stemmer.Stem("mules"));
            Assert.AreEqual("deni", stemmer.Stem("denied"));
            Assert.AreEqual("die", stemmer.Stem("died"));
            Assert.AreEqual("agre", stemmer.Stem("agreed"));
            Assert.AreEqual("own", stemmer.Stem("owned"));
            Assert.AreEqual("humbl", stemmer.Stem("humbled"));
            Assert.AreEqual("size", stemmer.Stem("sized"));
            Assert.AreEqual("meet", stemmer.Stem("meeting"));
            Assert.AreEqual("state", stemmer.Stem("stating"));
            Assert.AreEqual("siez", stemmer.Stem("siezing"));
            Assert.AreEqual("item", stemmer.Stem("itemization"));
            Assert.AreEqual("sensat", stemmer.Stem("sensational"));
            Assert.AreEqual("tradit", stemmer.Stem("traditional"));
            Assert.AreEqual("refer", stemmer.Stem("reference"));
            Assert.AreEqual("colon", stemmer.Stem("colonizer"));
            Assert.AreEqual("plot", stemmer.Stem("plotted"));
        }

        [Test]
        public void English_ShortString()
        {
            EnglishStemmer stemmer = new EnglishStemmer();

            Assert.AreEqual("y", stemmer.Stem("y's"));
        }

        [Test]
        public void English_FullTest()
        {
            Tools.Test(new EnglishStemmer(), "english");
        }
    }
}
