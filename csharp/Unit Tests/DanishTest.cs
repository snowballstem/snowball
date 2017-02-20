using System;
using NUnit.Framework;
using Snowball;
using Unit_Tests.Properties;

namespace Unit_Tests
{
    [TestFixture]
    public class DanishTest
    {
        [Test]
        public void Danish_BaseTest()
        {
            DanishStemmer stemmer = new DanishStemmer();

            Assert.AreEqual("abrænd", stemmer.Stem("abrændes"));
            Assert.AreEqual("barnløs", stemmer.Stem("barnløst"));
        }

        [Test]
        public void Danish_FullTest()
        {
            Tools.Test(new DanishStemmer(), "danish");
        }

    }
}
