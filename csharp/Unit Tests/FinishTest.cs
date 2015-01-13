using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;
using Unit_Tests.Properties;

namespace Unit_Tests
{
    [TestClass]
    public class FinnishTest
    {
        [TestMethod]
        public void Finnish_BaseTest()
        {
            var stemmer = new FinnishStemmer();

            Assert.AreEqual("aachen", stemmer.Stem("aachenin"));
        }

        [TestMethod]
        public void Danish_FullTest()
        {
            Tools.Test(new FinnishStemmer(), "finnish");
        }

    }
}
