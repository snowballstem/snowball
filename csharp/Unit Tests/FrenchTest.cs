using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Snowball;

namespace Unit_Tests
{
    [TestClass]
    public class FrenchTest
    {
        [TestMethod]
        public void BaseTest_NormalPath1()
        {
            FrenchStemmer stemmer = new FrenchStemmer();

            string[] inputs = 
            {
                "continu", "continua", "continuait", "continuant", "continuation", "continue",
                "continué", "continuel", "continuelle", "continuellement", "continuelles",
                "continuels", "continuer", "continuera", "continuerait", "continueront",
                "continuez", "continuité", "continuons", "contorsions", "contour", "contournait",
                "contournant", "contourne", "contours", "contractait", "contracté", "contractée",
                "contracter", "contractés", "contractions", "contradictoirement", "contradictoires",
                "contraindre", "contraint", "contrainte", "contraintes", "contraire", "contraires",
                "contraria"
            };

            string[] stemmed =
            {
                "continu", "continu", "continu", "continu", "continu", "continu", "continu", 
                "continuel", "continuel", "continuel", "continuel", "continuel", "continu", 
                "continu", "continu", "continu", "continu", "continu", "continuon", "contors", 
                "contour", "contourn", "contourn", "contourn", "contour", "contract", "contract", 
                "contract", "contract", "contract", "contract", "contradictoir", "contradictoir", 
                "contraindr", "contraint", "contraint", "contraint", "contrair", "contrair", "contrari", 
            };

            for (int i = 0; i < inputs.Length; i++)
            {
                string expected = stemmed[i];
                string actual = stemmer.Stem(inputs[i]);
                Assert.AreEqual(expected, actual);
            }
        }

        [TestMethod]
        public void BaseTest_NormalPath2()
        {
            FrenchStemmer stemmer = new FrenchStemmer();

            string[] inputs = 
            {
                "main", "mains", "maintenaient", "maintenait", "maintenant", "maintenir", "maintenue",
                "maintien", "maintint", "maire", "maires", "mairie", "mais", "maïs", "maison", "maisons",
                "maistre", "maitre", "maître", "maîtres", "maîtresse", "maîtresses", "majesté", "majestueuse",
                "majestueusement", "majestueux", "majeur", "majeure", "major", "majordome", "majordomes",
                "majorité", "majorités", "mal", "malacca", "malade", "malades", "maladie", "maladies", "maladive"
            };

            string[] stemmed =
            {
                "main", "main", "mainten", "mainten", "mainten", "mainten", "maintenu", "maintien", "maintint",
                "mair", "mair", "mair", "mais", "maï", "maison", "maison", "maistr", "maitr", "maîtr", "maîtr", 
                "maîtress", "maîtress", "majest", "majestu", "majestu", "majestu", "majeur", "majeur", "major",
                "majordom", "majordom", "major", "major", "mal", "malacc", "malad", "malad", "malad", "malad", 
                "malad"
            };

            for (int i = 0; i < inputs.Length; i++)
            {
                string expected = stemmed[i];
                string actual = stemmer.Stem(inputs[i]);
                Assert.AreEqual(expected, actual);
            }
        }

        [TestMethod]
        public void AccentTest()
        {
            FrenchStemmer stemmer = new FrenchStemmer();

            string actual = stemmer.Stem("majesté");
            string expected = "majest";

            Assert.AreEqual(expected, actual);
        }
    }
}
