using System;
using System.Text;

namespace Snowball
{
    public sealed class Among : IComparable<Among>
    {


        public string s;          // search string
        public int substring_i;   // index to longest matching substring 
        public int result;        // result of the lookup
        public Func<int> action;  // object to invoke method on 

        public Among(String s, int substring_i, int result)
            : this(s, substring_i, result, null)
        {
        }

        public Among(String s, int substring_i, int result, Func<int> methodobject)
        {
            this.s = s;//s.ToCharArray();
            this.substring_i = substring_i;
            this.result = result;
            this.action = methodobject;
        }

        public override string ToString()
        {
            return s;//new String(s);
        }




        public int CompareTo(Among other)
        {
            return s.CompareTo(other.s);
        }

    }
}
