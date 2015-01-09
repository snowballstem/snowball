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
            string utf16 = UTF8ToUTF16(s);

            this.s = utf16.ToCharArray();
            this.substring_i = substring_i;
            this.result = result;
            this.action = methodobject;
        }

        public override string ToString()
        {
            return new String(s);
        }


        public static string UTF8ToUTF16(string utf8String)
        {
            // Get UTF8 bytes by reading each byte with ANSI encoding
            byte[] utf8Bytes = Encoding.Default.GetBytes(utf8String);

            // Convert UTF8 bytes to UTF16 bytes
            byte[] utf16Bytes = Encoding.Convert(Encoding.UTF8, Encoding.Unicode, utf8Bytes);

            // Return UTF16 bytes as UTF16 string
            return Encoding.Unicode.GetString(utf16Bytes);
        }
    }
}
