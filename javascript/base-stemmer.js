// @ts-check

class BaseStemmer {
    constructor() {
        /** @protected */
        this.current = '';
        this.cursor = 0;
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
        this.cursor = 0;
        this.limit = this.current.length;
        this.limit_backward = 0;
        this.bra = this.cursor;
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
        this.cursor           = other.cursor;
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
        if (this.cursor >= this.limit) return false;
        let ch = this.current.charCodeAt(this.cursor);
        if (ch > max || ch < min) return false;
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) return false;
        this.cursor++;
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
        while (this.cursor < this.limit) {
            let ch = this.current.charCodeAt(this.cursor);
            if (ch > max || ch < min)
                return true;
            ch -= min;
            if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0)
                return true;
            this.cursor++;
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
        if (this.cursor <= this.limit_backward) return false;
        let ch = this.current.charCodeAt(this.cursor - 1);
        if (ch > max || ch < min) return false;
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) return false;
        this.cursor--;
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
        while (this.cursor > this.limit_backward) {
            let ch = this.current.charCodeAt(this.cursor - 1);
            if (ch > max || ch < min) return true;
            ch -= min;
            if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) return true;
            this.cursor--;
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
        if (this.cursor >= this.limit) return false;
        let ch = this.current.charCodeAt(this.cursor);
        if (ch > max || ch < min) {
            this.cursor++;
            return true;
        }
        ch -= min;
        if ((s[ch >>> 3] & (0X1 << (ch & 0x7))) === 0) {
            this.cursor++;
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
        while (this.cursor < this.limit) {
            let ch = this.current.charCodeAt(this.cursor);
            if (ch <= max && ch >= min) {
                ch -= min;
                if ((s[ch >>> 3] & (0X1 << (ch & 0x7))) !== 0) {
                    return true;
                }
            }
            this.cursor++;
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
        if (this.cursor <= this.limit_backward) return false;
        let ch = this.current.charCodeAt(this.cursor - 1);
        if (ch > max || ch < min) {
            this.cursor--;
            return true;
        }
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) {
            this.cursor--;
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
        while (this.cursor > this.limit_backward) {
            let ch = this.current.charCodeAt(this.cursor - 1);
            if (ch <= max && ch >= min) {
                ch -= min;
                if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) !== 0) {
                    return true;
                }
            }
            this.cursor--;
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
        if (this.limit - this.cursor < s.length) return false;
        if (this.current.slice(this.cursor, this.cursor + s.length) !== s)
        {
            return false;
        }
        this.cursor += s.length;
        return true;
    }

    /**
     * @param {string} s
     * @return {boolean}
     */
    eq_s_b(s)
    {
        /** @protected */
        if (this.cursor - this.limit_backward < s.length) return false;
        if (this.current.slice(this.cursor - s.length, this.cursor) !== s)
        {
            return false;
        }
        this.cursor -= s.length;
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

        const c = this.cursor;
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
            for (i2 = common; i2 < w[0].length; i2++)
            {
                if (c + common === l)
                {
                    diff = -1;
                    break;
                }
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
            if (common_i >= w[0].length)
            {
                this.cursor = c + w[0].length;
                if (w.length < 4) return w[2];
                this.af = w[3];
                if (call_among_func.call(this))
                {
                    this.cursor = c + w[0].length;
                    return w[2];
                }
            }
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

        const c = this.cursor;
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
            for (i2 = w[0].length - 1 - common; i2 >= 0; i2--)
            {
                if (c - common === lb)
                {
                    diff = -1;
                    break;
                }
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
            if (common_i >= w[0].length)
            {
                this.cursor = c - w[0].length;
                if (w.length < 4) return w[2];
                this.af = w[3];
                if (call_among_func.call(this))
                {
                    this.cursor = c - w[0].length;
                    return w[2];
                }
            }
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
        if (this.cursor >= c_ket) this.cursor += adjustment;
        else if (this.cursor > c_bra) this.cursor = c_bra;
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
