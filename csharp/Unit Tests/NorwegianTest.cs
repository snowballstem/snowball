using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class NorwegianTest
    {
        [Test]
        public void Norwegian_FullTest()
        {
            Tools.Test(new NorwegianStemmer(), "norwegian");
        }
    }
}
