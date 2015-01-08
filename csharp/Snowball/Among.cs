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

        public Among(String s, int substring_i, int result)
            : this(s, substring_i, result, null)
        {
        }

        public Among(String s, int substring_i, int result, Func<bool> methodobject)
        {
            this.s_size = s.Length;
            this.s = s.ToCharArray();
            this.substring_i = substring_i;
            this.result = result;
            this.methodobject = methodobject;
        }

    }
}
