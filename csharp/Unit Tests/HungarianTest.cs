using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class HungarianTest
    {
        [TestMethod]
        public void Hungarian_BaseTest()
        {
            var hungarian = new HungarianStemmer();

            Assert.AreEqual("ab", hungarian.Stem("abból"));
        }

        [TestMethod]
        public void Hungarian_DoubleAcuteTest()
        {
            var hungarian = new HungarianStemmer();

            Assert.AreEqual("bőgőz", hungarian.Stem("bőgőzik"));
        }

        [TestMethod]
        public void Hungarian_HyphenTest()
        {
            var hungarian = new HungarianStemmer();

            Assert.AreEqual("adattárház-menedzser", hungarian.Stem("adattárház-menedzsertől"));
        }

        [TestMethod]
        public void Hungarian_FullTest()
        {
            Tools.Test(new HungarianStemmer(), "hungarian");
        }
    }
}
