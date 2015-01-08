using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Snowball
{

    public abstract class SnowballStemmer
    {
        public abstract bool Stem();

        protected static Func<bool> methodObject;

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
            setCurrent("");
        }

        /// <summary>
        ///   Set the current string.
        /// </summary>
        /// 
        public void setCurrent(String value)
        {
            current.Clear();
            current.Insert(0, value);
            cursor = 0;
            limit = current.Length;
            limit_backward = 0;
            bra = cursor;
            ket = limit;
        }

        /**
         * Get the current string.
         */
        public String getCurrent()
        {
            String result = current.ToString();

            // Make a new StringBuffer. If we reuse the old one, and a user of
            // the library keeps a reference to the buffer returned (for example,
            // by converting it to a String in a way which doesn't force a copy),
            // the buffer size will not decrease, and we will risk wasting a large
            // amount of memory.

            // Thanks to Wolfram Esser for spotting this problem.
            current = new StringBuilder();
            return result;
        }



        protected bool in_grouping(int[] s, int min, int max)
        {
            if (cursor >= limit)
                return false;

            char ch = current[cursor];
            if (ch > max || ch < min)
                return false;

            ch = (char)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
                return false;

            cursor++;
            return true;
        }

        protected bool in_grouping_b(int[] s, int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            char ch = current[cursor - 1];
            if (ch > max || ch < min)
                return false;

            ch = (char)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
                return false;

            cursor--;
            return true;
        }

        protected bool out_grouping(int[] s, int min, int max)
        {
            if (cursor >= limit)
                return false;
            char ch = current[cursor];
            if (ch > max || ch < min)
            {
                cursor++;
                return true;
            }

            ch = (char)(ch - min);
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

            char ch = current[cursor - 1];
            if (ch > max || ch < min)
            {
                cursor--;
                return true;
            }

            ch = (char)(ch - min);
            if ((s[ch >> 3] & (0X1 << (ch & 0X7))) == 0)
            {
                cursor--;
                return true;
            }
            return false;
        }

        protected bool in_range(int min, int max)
        {
            if (cursor >= limit) return false;
            char ch = current[cursor];
            if (ch > max || ch < min) return false;
            cursor++;
            return true;
        }

        protected bool in_range_b(int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            char ch = current[cursor - 1];
            if (ch > max || ch < min)
                return false;

            cursor--;
            return true;
        }

        protected bool out_range(int min, int max)
        {
            if (cursor >= limit)
                return false;
            char ch = current[cursor];
            if (!(ch > max || ch < min))
                return false;

            cursor++;
            return true;
        }

        protected bool out_range_b(int min, int max)
        {
            if (cursor <= limit_backward)
                return false;

            char ch = current[cursor - 1];
            if (!(ch > max || ch < min))
                return false;

            cursor--;
            return true;
        }

        protected bool eq_s(int s_size, String s)
        {
            if (limit - cursor < s_size)
                return false;

            for (int i = 0; i != s_size; i++)
            {
                if (current[cursor + i] != s[i])
                    return false;
            }

            cursor += s_size;
            return true;
        }

        protected bool eq_s_b(int s_size, String s)
        {
            if (cursor - limit_backward < s_size)
                return false;

            for (int i = 0; i != s_size; i++)
            {
                if (current[cursor - s_size + i] != s[i])
                    return false;
            }

            cursor -= s_size;
            return true;
        }

        protected bool eq_s_b(int s_size, StringBuilder s)
        {
            if (cursor - limit_backward < s_size)
                return false;

            for (int i = 0; i != s_size; i++)
            {
                if (current[cursor - s_size + i] != s[i])
                    return false;
            }

            cursor -= s_size;
            return true;
        }

        protected bool eq_v(string s)
        {
            return eq_s(s.Length, s);
        }

        protected bool eq_v_b(StringBuilder s)
        {
            return eq_s_b(s.Length, s);
        }

        protected bool eq_v_b(string s)
        {
            return eq_s_b(s.Length, s);
        }

        protected int find_among(Among[] v, int v_size)
        {
            int i = 0;
            int j = v_size;

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
                for (i2 = common; i2 < w.s_size; i2++)
                {
                    if (c + common == l)
                    {
                        diff = -1;
                        break;
                    }
                    diff = current[c + common] - w.s[i2];
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
                if (common_i >= w.s_size)
                {
                    cursor = c + w.s_size;

                    if (w.methodobject == null)
                        return w.result;

                    bool res = false;

                    res = w.methodobject();

                    cursor = c + w.s_size;

                    if (res)
                        return w.result;
                }
                i = w.substring_i;
                if (i < 0) return 0;
            }
        }

        // find_among_b is for backwards processing. Same comments apply
        protected int find_among_b(Among[] v, int v_size)
        {
            int i = 0;
            int j = v_size;

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
                for (i2 = w.s_size - 1 - common; i2 >= 0; i2--)
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
                    if (i > 0) break;
                    if (j == i) break;
                    if (first_key_inspected) break;
                    first_key_inspected = true;
                }
            }
            while (true)
            {
                Among w = v[i];
                if (common_i >= w.s_size)
                {
                    cursor = c - w.s_size;
                    if (w.methodobject == null)
                        return w.result;

                    bool res = w.methodobject();

                    cursor = c - w.s_size;

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

        /// <summary>
        ///   Copy the slice into the supplied StringBuilder
        /// </summary>
        /// 
        protected StringBuilder slice_to(StringBuilder s)
        {
            slice_check();
            int len = ket - bra;
            Replace(s, 0, s.Length, current.ToString(bra, ket));
            return s;
        }

        public static StringBuilder Replace(StringBuilder sb, int index, int length, string text)
        {
            sb.Remove(index, length);
            sb.Insert(0, text);
            return sb;
        }

        protected StringBuilder assign_to(StringBuilder s)
        {
            Replace(s, 0, s.Length, current.ToString(0, limit));
            return s;
        }


    }
}
