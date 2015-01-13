using System;
using NUnit.Framework;
using Snowball;
using Unit_Tests.Properties;

namespace Unit_Tests
{
    [TestFixture]
    public class DutchTest
    {
        [Test]
        public void Dutch_FullTest()
        {
            Tools.Test(new DutchStemmer(), "dutch");
        }

    }
}
