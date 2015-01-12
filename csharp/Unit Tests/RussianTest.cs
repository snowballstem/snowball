using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class RussianTest
    {
        [TestMethod]
        public void Russian_BaseTest()
        {
            var stemmer = new RussianStemmer();

            Assert.AreEqual("абиссин", stemmer.Stem("абиссинию"));
        }


        [TestMethod]
        public void Russian_FullTest()
        {
            Tools.Test(new RussianStemmer(), "russian");
        }
    }
}
