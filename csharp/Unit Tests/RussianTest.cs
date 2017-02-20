using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class RussianTest
    {
        [Test]
        public void Russian_BaseTest()
        {
            var stemmer = new RussianStemmer();

            Assert.AreEqual("абиссин", stemmer.Stem("абиссинию"));
        }


        [Test]
        public void Russian_FullTest()
        {
            Tools.Test(new RussianStemmer(), "russian");
        }
    }
}
