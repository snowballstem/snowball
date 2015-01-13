using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class PorterTest
    {
        [Test]
        public void Porter_FullTest()
        {
            Tools.Test(new PorterStemmer(), "porter");
        }
    }
}
