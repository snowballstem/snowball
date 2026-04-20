// @ts-check

class BaseStemmer {
    constructor() {
        /** @protected */
        this.current = '';
        this.c = 0;
        this.limit = 0;
        this.limit_backward = 0;
        this.bra = 0;
        this.ket = 0;
        this.af = 0;
    }

    /**
     * @param {string} value
     */
    setCurrent(value) {
        this.current = value;
        this.c = 0;
        this.limit = this.current.length;
        this.limit_backward = 0;
        this.bra = this.c;
        this.ket = this.limit;
    }

    /**
     * @return {string}
     */
    getCurrent() {
        return this.current;
    }

    /**
     * @param {BaseStemmer} other
     */
    copy_from(other) {
        /** @protected */
        this.current          = other.current;
        this.c                = other.c;
        this.limit            = other.limit;
        this.limit_backward   = other.limit_backward;
        this.bra              = other.bra;
        this.ket              = other.ket;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    in_grouping(s, min, max) {
        /** @protected */
        if (this.c >= this.limit) return false;
        let ch = this.current.charCodeAt(this.c);
        if (ch > max || ch < min) return false;
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) return false;
        this.c++;
        return true;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    go_in_grouping(s, min, max) {
        /** @protected */
        while (this.c < this.limit) {
            let ch = this.current.charCodeAt(this.c);
            if (ch > max || ch < min)
                return true;
            ch -= min;
            if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0)
                return true;
            this.c++;
        }
        return false;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    in_grouping_b(s, min, max) {
        /** @protected */
        if (this.c <= this.limit_backward) return false;
        let ch = this.current.charCodeAt(this.c - 1);
        if (ch > max || ch < min) return false;
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) return false;
        this.c--;
        return true;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    go_in_grouping_b(s, min, max) {
        /** @protected */
        while (this.c > this.limit_backward) {
            let ch = this.current.charCodeAt(this.c - 1);
            if (ch > max || ch < min) return true;
            ch -= min;
            if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) return true;
            this.c--;
        }
        return false;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    out_grouping(s, min, max) {
        /** @protected */
        if (this.c >= this.limit) return false;
        let ch = this.current.charCodeAt(this.c);
        if (ch > max || ch < min) {
            this.c++;
            return true;
        }
        ch -= min;
        if ((s[ch >>> 3] & (0X1 << (ch & 0x7))) === 0) {
            this.c++;
            return true;
        }
        return false;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    go_out_grouping(s, min, max) {
        /** @protected */
        while (this.c < this.limit) {
            let ch = this.current.charCodeAt(this.c);
            if (ch <= max && ch >= min) {
                ch -= min;
                if ((s[ch >>> 3] & (0X1 << (ch & 0x7))) !== 0) {
                    return true;
                }
            }
            this.c++;
        }
        return false;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    out_grouping_b(s, min, max) {
        /** @protected */
        if (this.c <= this.limit_backward) return false;
        let ch = this.current.charCodeAt(this.c - 1);
        if (ch > max || ch < min) {
            this.c--;
            return true;
        }
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) {
            this.c--;
            return true;
        }
        return false;
    }

    /**
     * @param {Array<number>} s
     * @param {number} min
     * @param {number} max
     * @return {boolean}
     */
    go_out_grouping_b(s, min, max) {
        /** @protected */
        while (this.c > this.limit_backward) {
            let ch = this.current.charCodeAt(this.c - 1);
            if (ch <= max && ch >= min) {
                ch -= min;
                if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) !== 0) {
                    return true;
                }
            }
            this.c--;
        }
        return false;
    }

    /**
     * @param {string} s
     * @return {boolean}
     */
    eq_s(s)
    {
        /** @protected */
        if (this.limit - this.c < s.length) return false;
        if (!this.current.startsWith(s, this.c))
        {
            return false;
        }
        this.c += s.length;
        return true;
    }

    /**
     * @param {string} s
     * @return {boolean}
     */
    eq_s_b(s)
    {
        /** @protected */
        if (this.c - this.limit_backward < s.length) return false;
        if (!this.current.endsWith(s, this.c))
        {
            return false;
        }
        this.c -= s.length;
        return true;
    }

    /**
     * @param {Array<Array<string|number>>} v
     * @param {?function(): boolean} call_among_func
     * @return {number}
     */
    find_among(v, call_among_func)
    {
        /** @protected */
        let i = 0;
        let j = v.length;

        const c = this.c;
        const l = this.limit;

        let common_i = 0;
        let common_j = 0;

        let first_key_inspected = false;

        while (true)
        {
            const k = i + ((j - i) >>> 1);
            let diff = 0;
            let common = common_i < common_j ? common_i : common_j; // smaller
            // w[0]: string, w[1]: substring_i, w[2]: result, w[3]: function (optional)
            const w = v[k];
            let i2;
            // @ts-expect-error: w[0] always string.
            for (i2 = common; i2 < w[0].length; i2++)
            {
                if (c + common === l)
                {
                    diff = -1;
                    break;
                }
                // @ts-expect-error: w[0] always string.
                diff = this.current.charCodeAt(c + common) - w[0].charCodeAt(i2);
                if (diff !== 0) break;
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
                if (j === i) break; // only one item in v

                // - but now we need to go round once more to get
                // v->s inspected. This looks messy, but is actually
                // the optimal approach.

                if (first_key_inspected) break;
                first_key_inspected = true;
            }
        }
        do {
            const w = v[i];
            // @ts-expect-error: w[0] always string.
            if (common_i >= w[0].length)
            {
                // @ts-expect-error: w[0] always string.
                this.c = c + w[0].length;
                // @ts-expect-error: w[2] always number.
                if (w.length < 4) return w[2];
                // @ts-expect-error: w[3] always number.
                this.af = w[3];
                // @ts-expect-error: call_among_func never null here.
                if (call_among_func.call(this))
                {
                    // @ts-expect-error: w[0] always string.
                    this.c = c + w[0].length;
                    // @ts-expect-error: w[3] always number.
                    return w[2];
                }
            }
            // @ts-expect-error: w[1] always number.
            i = w[1];
        } while (i >= 0);
        return 0;
    }

    // find_among_b is for backwards processing. Same comments apply
    /**
     * @param {Array<Array<string|number>>} v
     * @param {?function(): boolean} call_among_func
     */
    find_among_b(v, call_among_func)
    {
        /** @protected */
        let i = 0;
        let j = v.length

        const c = this.c;
        const lb = this.limit_backward;

        let common_i = 0;
        let common_j = 0;

        let first_key_inspected = false;

        while (true)
        {
            const k = i + ((j - i) >> 1);
            let diff = 0;
            let common = common_i < common_j ? common_i : common_j;
            const w = v[k];
            let i2;
            // @ts-expect-error: w[0] always string.
            for (i2 = w[0].length - 1 - common; i2 >= 0; i2--)
            {
                if (c - common === lb)
                {
                    diff = -1;
                    break;
                }
                // @ts-expect-error: w[0] always string.
                diff = this.current.charCodeAt(c - 1 - common) - w[0].charCodeAt(i2);
                if (diff !== 0) break;
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
                if (j === i) break;
                if (first_key_inspected) break;
                first_key_inspected = true;
            }
        }
        do {
            const w = v[i];
            // @ts-expect-error: w[0] always string.
            if (common_i >= w[0].length)
            {
                // @ts-expect-error: w[0] always string.
                this.c = c - w[0].length;
                if (w.length < 4) return w[2];
                // @ts-expect-error: w[3] always number.
                this.af = w[3];
                // @ts-expect-error: call_among_func never null here.
                if (call_among_func.call(this))
                {
                    // @ts-expect-error: w[0] always string.
                    this.c = c - w[0].length;
                    return w[2];
                }
            }
            // @ts-expect-error: w[1] always number.
            i = w[1];
        } while (i >= 0);
        return 0;
    }

    /* to replace chars between c_bra and c_ket in this.current by the
     * chars in s.
     */
    /**
     * @param {number} c_bra
     * @param {number} c_ket
     * @param {string} s
     * @return {number}
     */
    #replace_s(c_bra, c_ket, s)
    {
        const adjustment = s.length - (c_ket - c_bra);
        this.current = this.current.slice(0, c_bra) + s + this.current.slice(c_ket);
        this.limit += adjustment;
        if (this.c >= c_ket) this.c += adjustment;
        else if (this.c > c_bra) this.c = c_bra;
        return adjustment;
    }

    /**
     */
    #slice_check()
    {
        console.assert(this.bra >= 0);
        console.assert(this.bra <= this.ket);
        console.assert(this.ket <= this.limit);
        console.assert(this.limit <= this.current.length);
    }

    /**
     * @param {string} s
     */
    slice_from(s)
    {
        /** @protected */
        this.#slice_check();
        this.#replace_s(this.bra, this.ket, s);
        this.ket = this.bra + s.length;
    }

    /**
     */
    slice_del()
    {
        /** @protected */
        this.slice_from("");
    }

    /**
     * @param {number} c_bra
     * @param {number} c_ket
     * @param {string} s
     */
    insert(c_bra, c_ket, s)
    {
        /** @protected */
        const adjustment = this.#replace_s(c_bra, c_ket, s);
        if (c_bra <= this.bra) this.bra += adjustment;
        if (c_bra <= this.ket) this.ket += adjustment;
    }

    /**
     * @return {string}
     */
    slice_to()
    {
        /** @protected */
        this.#slice_check();
        return this.current.slice(this.bra, this.ket);
    }
}

export { BaseStemmer };
