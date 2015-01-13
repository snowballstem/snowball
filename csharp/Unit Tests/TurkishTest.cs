using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class TurkishTest
    {
        [Test]
        public void Turkish_FullTest()
        {
            Tools.Test(new TurkishStemmer(), "turkish");
        }
    }
}
