using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class HungarianTest
    {
        [Test]
        public void Hungarian_BaseTest()
        {
            var hungarian = new HungarianStemmer();

            Assert.AreEqual("ab", hungarian.Stem("abból"));
        }

        [Test]
        public void Hungarian_DoubleAcuteTest()
        {
            var hungarian = new HungarianStemmer();

            Assert.AreEqual("bőgőz", hungarian.Stem("bőgőzik"));
        }

        [Test]
        public void Hungarian_HyphenTest()
        {
            var hungarian = new HungarianStemmer();

            Assert.AreEqual("adattárház-menedzser", hungarian.Stem("adattárház-menedzsertől"));
        }

        [Test]
        public void Hungarian_FullTest()
        {
            Tools.Test(new HungarianStemmer(), "hungarian");
        }
    }
}
