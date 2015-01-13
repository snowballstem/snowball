using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class PortugueseTest
    {
        [Test]
        public void Portuguese_FullTest()
        {
            Tools.Test(new PortugueseStemmer(), "portuguese");
        }
    }
}
