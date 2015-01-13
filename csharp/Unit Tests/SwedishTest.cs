using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class SwedishTest
    {
        [Test]
        public void Swedish_FullTest()
        {
            Tools.Test(new SwedishStemmer(), "swedish");
        }
    }
}
