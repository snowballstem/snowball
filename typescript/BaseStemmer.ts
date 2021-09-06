export abstract class BaseStemmer {
    protected current: string;
    protected cursor: number;
    protected limit: number;
    protected limitBackward: number;
    protected bra: number;
    protected ket: number;

    protected constructor() {
        this.current = '';
        this.cursor = 0;
        this.limit = 0;
        this.limitBackward = 0;
        this.bra = 0;
        this.ket = 0;
    }

    abstract stemWord(word: string): string;

    protected setCurrent(value: string) {
        this.current = value;
        this.cursor = 0;
        this.limit = this.current.length;
        this.limitBackward = 0;
        this.bra = this.cursor;
        this.ket = this.limit;
    }

    protected copyFrom(other: BaseStemmer) {
        this.current = other.current;
        this.cursor = other.cursor;
        this.limit = other.limit;
        this.limitBackward = other.limitBackward;
        this.bra = other.bra;
        this.ket = other.ket;
    }

    protected inGrouping(s: number[], min: number, max: number): boolean {
        if (this.cursor >= this.limit) {
            return false;
        }
        let ch = this.current.charCodeAt(this.cursor);
        if (ch > max || ch < min) {
            return false;
        }
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) {
            return false;
        }
        this.cursor++;
        return true;
    }

    protected inGroupingBackward(s: number[], min: number, max: number): boolean {
        if (this.cursor <= this.limitBackward) {
            return false;
        }
        let ch = this.current.charCodeAt(this.cursor - 1);
        if (ch > max || ch < min) {
            return false;
        }
        ch -= min;
        if ((s[ch >>> 3] & (0x1 << (ch & 0x7))) === 0) {
            return false;
        }
        this.cursor--;
        return true;
    }

    protected outGrouping(s: number[], min: number, max: number): boolean {
        if (this.cursor >= this.limit) {
            return false;
        }
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

    protected outGroupingBackward(s: number[], min: number, max: number): boolean {
        if (this.cursor <= this.limitBackward) {
            return false;
        }
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

    protected eq(s: string): boolean {
        if (this.limit - this.cursor < s.length) {
            return false;
        }
        if (this.current.slice(this.cursor, this.cursor + s.length) !== s) {
            return false;
        }
        this.cursor += s.length;
        return true;
    }

    protected eqBackward(s: string): boolean {
        if (this.cursor - this.limitBackward < s.length) {
            return false;
        }
        if (this.current.slice(this.cursor - s.length, this.cursor) !== s) {
            return false;
        }
        this.cursor -= s.length;
        return true;
    }

    protected findAmong(v: any): number {
        let i = 0;
        let j = v.length;

        let c = this.cursor;
        let l = this.limit;

        let commonI = 0;
        let commonJ = 0;

        let firstKeyInspected = false;

        while (true) {
            let k = i + ((j - i) >>> 1);
            let diff = 0;
            let common = commonI < commonJ ? commonI : commonJ; // smaller
            // w[0]: string, w[1]: substring_i, w[2]: result, w[3]: function (optional)
            let w = v[k];
            let i2;
            for (i2 = common; i2 < w[0].length; i2++) {
                if (c + common === l) {
                    diff = -1;
                    break;
                }
                diff = this.current.charCodeAt(c + common) - w[0].charCodeAt(i2);
                if (diff !== 0) {
                    break;
                }
                common++;
            }
            if (diff < 0) {
                j = k;
                commonJ = common;
            } else {
                i = k;
                commonI = common;
            }
            if (j - i <= 1) {
                if (i > 0) {
                    break; // v->s has been inspected
                }
                if (j === i) {
                    break; // only one item in v
                }
                // - but now we need to go round once more to get
                // v->s inspected. This looks messy, but is actually
                // the optimal approach.

                if (firstKeyInspected) {
                    break;
                }
                firstKeyInspected = true;
            }
        }
        do {
            let w = v[i];
            if (commonI >= w[0].length) {
                this.cursor = c + w[0].length;
                if (w.length < 4) {
                    return w[2];
                }
                let res = w[3](this);
                this.cursor = c + w[0].length;
                if (res) {
                    return w[2];
                }
            }
            i = w[1];
        } while (i >= 0);
        return 0;
    }

    // findAmongBackward is for backwards processing. Same comments apply
    protected findAmongBackward(v: any): number {
        let i = 0;
        let j = v.length;

        let c = this.cursor;
        let lb = this.limitBackward;

        let commonI = 0;
        let commonJ = 0;

        let firstKeyInspected = false;

        while (true) {
            let k = i + ((j - i) >> 1);
            let diff = 0;
            let common = commonI < commonJ ? commonI : commonJ;
            let w = v[k];
            let i2;
            for (i2 = w[0].length - 1 - common; i2 >= 0; i2--) {
                if (c - common === lb) {
                    diff = -1;
                    break;
                }
                diff = this.current.charCodeAt(c - 1 - common) - w[0].charCodeAt(i2);
                if (diff !== 0) {
                    break;
                }
                common++;
            }
            if (diff < 0) {
                j = k;
                commonJ = common;
            } else {
                i = k;
                commonI = common;
            }
            if (j - i <= 1) {
                if (i > 0) {
                    break;
                }
                if (j === i) {
                    break;
                }
                if (firstKeyInspected) {
                    break;
                }
                firstKeyInspected = true;
            }
        }
        do {
            let w = v[i];
            if (commonI >= w[0].length) {
                this.cursor = c - w[0].length;
                if (w.length < 4) {
                    return w[2];
                }
                let res = w[3](this);
                this.cursor = c - w[0].length;
                if (res) {
                    return w[2];
                }
            }
            i = w[1];
        } while (i >= 0);
        return 0;
    }

    // To replace chars between bra and ket in this.current by the chars in s.
    protected replace(bra: number, ket: number, s: string): number {
        let adjustment = s.length - (ket - bra);
        this.current = this.current.slice(0, bra) + s + this.current.slice(ket);
        this.limit += adjustment;
        if (this.cursor >= ket) {
            this.cursor += adjustment;
        } else if (this.cursor > bra) {
            this.cursor = bra;
        }
        return adjustment;
    }

    protected sliceCheck(): boolean {
        if (this.bra < 0 ||
            this.bra > this.ket ||
            this.ket > this.limit ||
            this.limit > this.current.length) {
            return false;
        }
        return true;
    }

    protected sliceFrom(s: string): boolean {
        let result = false;
        if (this.sliceCheck()) {
            this.replace(this.bra, this.ket, s);
            result = true;
        }
        return result;
    }

    protected sliceDel(): boolean {
        return this.sliceFrom("");
    }

    protected insert(bra: number, ket: number, s: string) {
        let adjustment = this.replace(bra, ket, s);
        if (bra <= this.bra) {
            this.bra += adjustment;
        }
        if (bra <= this.ket) {
            this.ket += adjustment;
        }
    }

    protected sliceTo(): string {
        let result = '';
        if (this.sliceCheck()) {
            result = this.current.slice(this.bra, this.ket);
        }
        return result;
    }

    protected assignTo(): string {
        return this.current.slice(0, this.limit);
    }
}
