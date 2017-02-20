using System;
using NUnit.Framework;
using Snowball;

namespace Unit_Tests
{
    [TestFixture]
    public class KraaijPohlmannTest
    {
        [Test]
        public void KraaijPohlmann_FullTest()
        {
            Tools.Test(new KraaijPohlmannStemmer(), "kraaij_pohlmann");
        }
    }
}
