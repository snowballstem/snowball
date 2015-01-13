using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class German2Test
    {
        [Test]
        public void German2_BaseTest()
        {
            var german = new German2Stemmer();

            Assert.AreEqual("amtsgeheimnis", german.Stem("amtsgeheimnisse"));
        }

        [Test, Ignore]
        public void German2_FullTest()
        {
            // This test is ignored because it seems that the german2 
            // output has not been updated after the -nis change:
            // http://comments.gmane.org/gmane.comp.search.snowball/1119

            Tools.Test(new German2Stemmer(), "german2");
        }
    }
}
