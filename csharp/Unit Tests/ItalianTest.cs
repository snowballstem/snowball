using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class ItalianTest
    {
        [Test]
        public void Italian_FullTest()
        {
            Tools.Test(new ItalianStemmer(), "italian");
        }
    }
}
