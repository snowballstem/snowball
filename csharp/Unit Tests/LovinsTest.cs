using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class LovinsTest
    {
        [Test]
        public void Lovins_FullTest()
        {
            Tools.Test(new LovinsStemmer(), "lovins");
        }
    }
}
