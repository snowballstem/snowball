using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class UnitTest1
    {
        [TestMethod]
        public void TestMethod1()
        {
            EnglishStemmer stemmer = new EnglishStemmer();

            stemmer.setCurrent("doing");

            stemmer.Stem();

            var actual = stemmer.getCurrent();

            Assert.AreEqual("do", actual);
        }
    }
}
