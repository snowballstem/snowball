using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class RomanianTest
    {
        [Test]
        public void Romanian_FullTest()
        {
            Tools.Test(new RomanianStemmer(), "romanian");
        }
    }
}
