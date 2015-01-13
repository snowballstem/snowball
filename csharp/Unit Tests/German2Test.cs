using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class German2Test
    {
        [TestMethod]
        public void German2_BaseTest()
        {
            var german = new German2Stemmer();

            Assert.AreEqual("amtsgeheimnis", german.Stem("amtsgeheimnisse"));
        }

        [TestMethod]
        public void German2_FullTest()
        {
            Tools.Test(new German2Stemmer(), "german2");
        }
    }
}
