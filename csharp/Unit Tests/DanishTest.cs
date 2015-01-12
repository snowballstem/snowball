using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;
using Unit_Tests.Properties;

namespace Unit_Tests
{
    [TestClass]
    public class DanishTest
    {
        [TestMethod]
        public void Danish_BaseTest()
        {
            DanishStemmer stemmer = new DanishStemmer();

            Assert.AreEqual("abrænd", stemmer.Stem("abrændes"));
            Assert.AreEqual("barnløs", stemmer.Stem("barnløst"));
        }

        [TestMethod]
        public void Danish_FullTest()
        {
            Tools.Test(new DanishStemmer(), "danish");
        }

    }
}
