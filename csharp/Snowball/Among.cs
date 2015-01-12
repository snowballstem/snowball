using System;
using System.Text;

namespace Snowball
{
    public sealed class Among
    {


        public char[] s;          // search string
        public int substring_i;   // index to longest matching substring 
        public int result;        // result of the lookup
        public Func<bool> action; // object to invoke method on 

        public Among(String s, int substring_i, int result)
            : this(s, substring_i, result, null)
        {
        }

        public Among(String s, int substring_i, int result, Func<bool> methodobject)
        {
            this.s = s.ToCharArray();
            this.substring_i = substring_i;
            this.result = result;
            this.action = methodobject;
        }

        public override string ToString()
        {
            return new String(s);
        }


        
    }
}
