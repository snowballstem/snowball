using System;
using NUnit.Framework;
using Snowball;
using Unit_Tests.Properties;

namespace Unit_Tests
{
    [TestFixture]
    public class FinnishTest
    {
        [Test]
        public void Finnish_BaseTest()
        {
            var stemmer = new FinnishStemmer();

            Assert.AreEqual("aachen", stemmer.Stem("aachenin"));
        }

        [Test]
        public void Danish_FullTest()
        {
            Tools.Test(new FinnishStemmer(), "finnish");
        }

    }
}
