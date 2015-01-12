using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Snowball
{

    public abstract class SnowballStemmer
    {
        // current string
        protected StringBuilder current;

        protected int cursor;
        protected int limit;
        protected int limit_backward;
        protected int bra;
        protected int ket;




        protected SnowballStemmer()
        {
            current = new StringBuilder();
            setBufferContents("");
        }



        protected abstract bool Process();


        public bool Stem()
        {
            return this.Process();
        }

        public string Stem(string word)
        {
            setBufferContents(word);
            this.Process();
            return current.ToString();
        }

        public string[] Stem(params string[] words)
        {
            string[] stemmed = new string[words.Length];
            for (int i = 0; i < stemmed.Length; i++)
            {
                setBufferContents(words[i]);
                this.Process();
                stemmed[i] = current.ToString();
            }

            return stemmed;
        }


        public StringBuilder Buffer
        {
            get { return current; }
        }

        public string Current
        {
            get { return current.ToString(); }
            set { setBufferContents(value); }
        }

        private void setBufferContents(string value)
        {
            current.Clear();
            current.Insert(0, value);

            cursor = 0;
            limit = current.Length;
            limit_backward = 0;
            bra = cursor;
            ket = limit;
        }



        protected bool in_grouping(int[] s, int min, int max)
        {
            if (cursor >= limit)
                return false;

            byte ch = (byte)current[cursor];
            if (ch > max || ch < min)
                return false;

            ch = (byte)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
                return false;

            cursor++;
            return true;
        }

        protected bool in_grouping_b(int[] s, int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            byte ch = (byte)current[cursor - 1];
            if (ch > max || ch < min)
                return false;

            ch = (byte)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
                return false;

            cursor--;
            return true;
        }

        protected bool out_grouping(int[] s, int min, int max)
        {
            if (cursor >= limit)
                return false;

            byte ch = (byte)current[cursor];
            if (ch > max || ch < min)
            {
                cursor++;
                return true;
            }

            ch = (byte)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
            {
                cursor++;
                return true;
            }
            return false;
        }

        protected bool out_grouping_b(int[] s, int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            byte ch = (byte)current[cursor - 1];
            if (ch > max || ch < min)
            {
                cursor--;
                return true;
            }

            ch = (byte)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
            {
                cursor--;
                return true;
            }
            return false;
        }

        protected bool in_range(int min, int max)
        {
            if (cursor >= limit) 
                return false;

            byte ch = (byte)current[cursor];

            if (ch > max || ch < min) 
                return false;

            cursor++;
            return true;
        }

        protected bool in_range_b(int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            byte ch = (byte)current[cursor - 1];
            if (ch > max || ch < min)
                return false;

            cursor--;
            return true;
        }

        protected bool out_range(int min, int max)
        {
            if (cursor >= limit)
                return false;

            byte ch = (byte)current[cursor];
            if (!(ch > max || ch < min))
                return false;

            cursor++;
            return true;
        }

        protected bool out_range_b(int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            byte ch = (byte)current[cursor - 1];
            if (!(ch > max || ch < min))
                return false;

            cursor--;
            return true;
        }


        protected bool eq_s(String s)
        {
            if (limit - cursor < s.Length)
                return false;

            for (int i = 0; i != s.Length; i++)
            {
                if (current[cursor + i] != s[i])
                    return false;
            }

            cursor += s.Length;
            return true;
        }

        protected bool eq_s_b(String s)
        {
            if (cursor - limit_backward < s.Length)
                return false;

            for (int i = 0; i != s.Length; i++)
            {
                if (current[cursor - s.Length + i] != s[i])
                    return false;
            }

            cursor -= s.Length;
            return true;
        }

        protected bool eq_s_b(StringBuilder s)
        {
            if (cursor - limit_backward < s.Length)
                return false;

            for (int i = 0; i != s.Length; i++)
            {
                if (current[cursor - s.Length + i] != s[i])
                    return false;
            }

            cursor -= s.Length;
            return true;
        }


        protected int find_among(Among[] v)
        {
            int i = 0;
            int j = v.Length;

            int c = cursor;
            int l = limit;

            int common_i = 0;
            int common_j = 0;

            bool first_key_inspected = false;

            while (true)
            {
                int k = i + ((j - i) >> 1);
                int diff = 0;
                int common = common_i < common_j ? common_i : common_j; // smaller
                Among w = v[k];
                int i2;
                for (i2 = common; i2 < w.s.Length; i2++)
                {
                    if (c + common == l)
                    {
                        diff = -1;
                        break;
                    }
                    diff = current[c + common] - w.s[i2];
                    if (diff != 0)
                        break;
                    common++;
                }
                if (diff < 0)
                {
                    j = k;
                    common_j = common;
                }
                else
                {
                    i = k;
                    common_i = common;
                }
                if (j - i <= 1)
                {
                    if (i > 0) break; // v->s has been inspected
                    if (j == i) break; // only one item in v

                    // - but now we need to go round once more to get
                    // v->s inspected. This looks messy, but is actually
                    // the optimal approach.

                    if (first_key_inspected) break;
                    first_key_inspected = true;
                }
            }
            while (true)
            {
                Among w = v[i];
                if (common_i >= w.s.Length)
                {
                    cursor = c + w.s.Length;

                    if (w.action == null)
                        return w.result;

                    bool res = false;

                    res = w.action();

                    cursor = c + w.s.Length;

                    if (res)
                        return w.result;
                }
                i = w.substring_i;
                if (i < 0) return 0;
            }
        }

        // find_among_b is for backwards processing. Same comments apply
        protected int find_among_b(Among[] v)
        {
            int i = 0;
            int j = v.Length;

            int c = cursor;
            int lb = limit_backward;

            int common_i = 0;
            int common_j = 0;

            bool first_key_inspected = false;

            while (true)
            {
                int k = i + ((j - i) >> 1);
                int diff = 0;
                int common = common_i < common_j ? common_i : common_j;
                Among w = v[k];
                int i2;
                for (i2 = w.s.Length - 1 - common; i2 >= 0; i2--)
                {
                    if (c - common == lb)
                    {
                        diff = -1;
                        break;
                    }
                    diff = current[c - 1 - common] - w.s[i2];
                    if (diff != 0) break;
                    common++;
                }
                if (diff < 0)
                {
                    j = k;
                    common_j = common;
                }
                else
                {
                    i = k;
                    common_i = common;
                }
                if (j - i <= 1)
                {
                    if (i > 0)
                        break;
                    if (j == i)
                        break;
                    if (first_key_inspected)
                        break;
                    first_key_inspected = true;
                }
            }
            while (true)
            {
                Among w = v[i];
                if (common_i >= w.s.Length)
                {
                    cursor = c - w.s.Length;
                    if (w.action == null)
                        return w.result;

                    bool res = w.action();

                    cursor = c - w.s.Length;

                    if (res)
                        return w.result;
                }

                i = w.substring_i;

                if (i < 0)
                    return 0;
            }
        }

        /* to replace chars between c_bra and c_ket in current by the
         * chars in s.
         */
        protected int replace_s(int c_bra, int c_ket, String s)
        {
            int adjustment = s.Length - (c_ket - c_bra);
            Replace(current, c_bra, c_ket, s);
            limit += adjustment;
            if (cursor >= c_ket)
                cursor += adjustment;
            else if (cursor > c_bra)
                cursor = c_bra;
            return adjustment;
        }

        protected void slice_check()
        {
            if (bra < 0 || bra > ket || ket > limit || limit > current.Length)
            {
                System.Diagnostics.Trace.WriteLine("faulty slice operation");
            }
        }

        protected void slice_from(String s)
        {
            slice_check();
            replace_s(bra, ket, s);
        }

        protected void slice_del()
        {
            slice_from("");
        }

        protected void insert(int c_bra, int c_ket, String s)
        {
            int adjustment = replace_s(c_bra, c_ket, s);
            if (c_bra <= bra) bra += adjustment;
            if (c_bra <= ket) ket += adjustment;
        }

        protected void insert(int c_bra, int c_ket, StringBuilder s)
        {
            int adjustment = replace_s(c_bra, c_ket, s.ToString());
            if (c_bra <= bra) bra += adjustment;
            if (c_bra <= ket) ket += adjustment;
        }

        protected StringBuilder slice_to(StringBuilder s)
        {
            slice_check();
            Replace(s, 0, s.Length, current.ToString(bra, ket - bra));
            return s;
        }

        protected StringBuilder assign_to(StringBuilder s)
        {
            Replace(s, 0, s.Length, current.ToString(0, limit));
            return s;
        }



        public static StringBuilder Replace(StringBuilder sb, int index, int length, string text)
        {
            sb.Remove(index, length - index);
            sb.Insert(index, text);
            return sb;
        }


        public static string UTF16(string utf8String)
        {
            // Get UTF8 bytes by reading each byte with ANSI encoding
            char[] chars = utf8String.ToCharArray();
            byte[] utf8Bytes = new byte[chars.Length];
            for (int i = 0; i < chars.Length; i++)
                utf8Bytes[i] = (byte)chars[i];

            // Convert UTF8 bytes to UTF16 bytes
            byte[] utf16Bytes = Encoding.Convert(Encoding.UTF8, Encoding.Unicode, utf8Bytes);

            // Return UTF16 bytes as UTF16 string
            string result = Encoding.Unicode.GetString(utf16Bytes);

            return result;
        }
    }
}
