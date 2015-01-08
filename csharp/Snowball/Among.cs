using System;
using System.Text;

namespace Snowball
{
    public class Among
    {

        public int s_size; /* search string */
        public char[] s; /* search string */
        public int substring_i; /* index to longest matching substring */
        public int result; /* result of the lookup */
        public Func<bool> methodobject; /* object to invoke method on */
        public string tag;

        public Among(String s, int substring_i, int result, string tag, Func<bool> methodobject)
        {
            this.s_size = s.Length;
            this.s = s.ToCharArray();
            this.substring_i = substring_i;
            this.result = result;
            this.methodobject = methodobject;
            this.tag = tag;
        }

    }
}
