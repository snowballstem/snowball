<?php
abstract class SnowballStemmer {

    protected string $current = '';
    protected int $cursor = 0;
    protected int $limit = 0;
    protected int $limit_backward = 0;
    protected int $bra = 0;
    protected int $ket = 0;
    
    
    abstract public function stem(): bool;

    
    protected function copyFrom(self $other): void {
        $this->current          = $other->current;
        $this->cursor           = $other->cursor;
        $this->limit            = $other->limit;
        $this->limit_backward   = $other->limit_backward;
        $this->bra              = $other->bra;
        $this->ket              = $other->ket;
    }


    /**
     * @param int[] $s
     */
    protected function in_grouping(array $s): bool {
        if ($this->cursor >= $this->limit) {
            return false;
        }
        $ch = $this->charAt();
        if (!array_key_exists($ch, $s)) {
            return false;
        }
        $this->cursor += strlen($ch);
        return true;
    }


    /**
     * @param int[] $s
     */
    protected function go_in_grouping(array $s): bool {
        while ($this->cursor < $this->limit) {
            $ch = $this->charAt();
            if (!array_key_exists($ch, $s)) {
                return true;
            }
            $this->cursor += strlen($ch);
        }
        return false;
    }



    /**
     * @param int[] $s
     */
    protected function in_grouping_b(array $s): bool {
        if ($this->cursor <= $this->limit_backward) {
            return false;
        }
        $ch = $this->charBefore();
        if (!array_key_exists($ch, $s)) {
            return false;
        }
        $this->cursor -= strlen($ch);
        return true;
    }


    /**
     * @param int[] $s
     */
    protected function go_in_grouping_b(array $s): bool {
        while ($this->cursor > $this->limit_backward) {
            $ch = $this->charBefore();
            if (!array_key_exists($ch, $s)) {
                return true;
            }
            $this->cursor -= strlen($ch);
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function out_grouping(array $s): bool {
        if ($this->cursor >= $this->limit) {
            return false;
        }
        $ch = $this->charAt();
        if (!array_key_exists($ch, $s)) {
            $this->cursor += strlen($ch);
            return true;
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function go_out_grouping(array $s): bool {
        while ($this->cursor < $this->limit) {
            $ch = $this->charAt();
            if (array_key_exists($ch, $s)) {
                return true;
            }
            $this->cursor += strlen($ch);
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function out_grouping_b(array $s): bool {
        if ($this->cursor <= $this->limit_backward) {
            return false;
        }
        $ch = $this->charBefore();
        if (!array_key_exists($ch, $s)) {
            $this->cursor -= strlen($ch);
            return true;
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function go_out_grouping_b(array $s): bool {
        while ($this->cursor > $this->limit_backward) {
            $ch = $this->charBefore();
            if (array_key_exists($ch, $s)) {
                return true;
            }
            $this->cursor -= strlen($ch);
        }
        return false;
    }


    protected function eq_s(string $s): bool {
        $slength = strlen($s);
        if ($this->limit - $this->cursor < $slength) {
            return false;
        }
        if (substr_compare($this->current, $s, $this->cursor, $slength) != 0) {
            return false;
        }
        $this->cursor += $slength;
        return true;
    }


    protected function eq_s_b(string $s): bool {
        $slength = strlen($s);
        if ($this->cursor - $this->limit_backward < $slength) {
            return false;
        }
        if (substr_compare($this->current, $s, $this->cursor - $slength, $slength) != 0) {
            return false;
        }
        $this->cursor -= $slength;
        return true;
    }


    /**
     * @param array[] $v
     */
    protected function find_among(array $v): int {
        $i = 0;
        $j = count($v);

        $c = $this->cursor;
        $l = $this->limit;

        $common_i = 0;
        $common_j = 0;

        $first_key_inspected = false;

        while (true) {
            $k = $i + (($j-$i) >> 1);
            $diff = 0;
            $common = min($common_i, $common_j); // smaller
            // w[0]: string, w[1]: substring_i, w[2]: result, w[3]: function (optional)
            $w = $v[$k];
            $w0length = strlen($w[0]);
            for ($i2 = $common; $i2 < $w0length; $i2++) {
                if ($c + $common === $l) {
                    $diff = -1;
                    break;
                }
                $diff = strcmp($this->current[$c+$common], $w[0][$i2]);
                if ($diff !== 0) {
                    break;
                }
                $common++;
            }
            if ($diff < 0) {
                $j = $k;
                $common_j = $common;
            }
            else {
                $i = $k;
                $common_i = $common;
            }
            if ($j - $i <= 1) {
                if ($i > 0) {
                    break;
                } 
                // v->s has been inspected
                if ($j === $i) {
                    break;
                } 
                // only one item in v

                // - but now we need to go round once more to get
                // v->s inspected. This looks messy, but is actually
                // the optimal approach.

                if ($first_key_inspected) {
                    break;
                }
                $first_key_inspected = true;
            }
        }
        do {
            $w = $v[$i];
            $w0length = strlen($w[0]);
            if ($common_i >= $w0length) {
                $this->cursor = $c + $w0length;
                if (count($w) < 4) {
                    return $w[2];
                }
                $res = $this->{$w[3]}();
                $this->cursor = $c + $w0length;
                if ($res) {
                    return $w[2];
                }
            }
            $i = $w[1];
        } while ($i >= 0);
        return 0;
    }


    /**
     * find_among_b is for backwards processing. Same comments apply
     */
    protected function find_among_b(array $v): int {
        $i = 0;
        $j = count($v);

        $c = $this->cursor;
        $lb = $this->limit_backward;

        $common_i = 0;
        $common_j = 0;

        $first_key_inspected = false;

        while (true) {
            $k = $i + (($j-$i) >> 1);
            $diff = 0;
            $common = min($common_i, $common_j);
            $w = $v[$k];
            $w0length = strlen($w[0]);
            for ($i2 = $w0length - 1 - $common; $i2 >= 0; $i2--) {
                if ($c - $common == $lb) {
                    $diff = -1;
                    break;
                }
                $diff = strcmp($this->current[$c - 1 - $common], $w[0][$i2]);
                if ($diff != 0) {
                    break;
                }
                $common++;
            }
            if ($diff < 0) {
                $j = $k;
                $common_j = $common;
            }
            else {
                $i = $k;
                $common_i = $common;
            }
            if ($j - $i <= 1) {
                if ($i > 0 || $j === $i || $first_key_inspected) {
                    break;
                }
                $first_key_inspected = true;
            }
        }
        do {
            $w = $v[$i];
            $w0length = strlen($w[0]);
            if ($common_i >= $w0length) {
                $this->cursor = $c - $w0length;
                if (count($w) < 4) {
                    return $w[2];
                }
                $res = $this->{$w[3]}();
                $this->cursor = $c - $w0length;
                if ($res) {
                    return $w[2];
                }
            }
            $i = $w[1];
        } while ($i >= 0);
        return 0;
    }


    /**
     * to replace chars between $c_bra and $c_ket in $this->current by the chars in $s.
     */
    private function replace_s(int $c_bra, int $c_ket, string $s): int {
        $slength = strlen($s);
        $adjustment = $slength - ($c_ket - $c_bra);
        $this->current = substr_replace($this->current, $s, $c_bra, $c_ket - $c_bra);
        $this->limit += $adjustment;
        if ($this->cursor >= $c_ket) {
            $this->cursor += $adjustment;
        } elseif ($this->cursor > $c_bra) {
            $this->cursor = $c_bra;
        }
        return $adjustment;
    }


    private function slice_check(): void {
        if (
            $this->bra < 0 ||
            $this->bra > $this->ket ||
            $this->ket > $this->limit ||
            $this->limit > strlen($this->current)
        ) {
            throw new LogicException('Faulty slice operation');
        }
    }


    protected function slice_from(string $s): void {
        $this->slice_check();
        $this->replace_s($this->bra, $this->ket, $s);
        $this->ket = $this->bra + strlen($s);
    }


    protected function slice_del(): void {
        $this->slice_from('');
    }


    protected function insert(int $c_bra, int $c_ket, string $s): void {
        $adjustment = $this->replace_s($c_bra, $c_ket, $s);
        $c_bra <= $this->bra and $this->bra += $adjustment;
        $c_bra <= $this->ket and $this->ket += $adjustment;
    }


    protected function slice_to(): string {
        $this->slice_check();
        return substr($this->current, $this->bra, $this->ket - $this->bra);
    }

    private function charAt(): string {
        $s = $this->current[$this->cursor];
        $c = ord($s);
        if ($c < 0xc0) return $s;
        if ($c < 0xe0) return substr($this->current, $this->cursor, 2);
        if ($c < 0xf0) return substr($this->current, $this->cursor, 3);
        return substr($this->current, $this->cursor, 4);
    }

    private function charBefore(): string {
        $s = $this->current[$this->cursor - 1];
        if (ord($s) < 0x80) return $s;
        $o = $this->cursor - 1;
        while (--$o && ord($this->current[$o]) < 0xc0) { }
        return substr($this->current, $o, $this->cursor - $o);
    }

    protected function inc_cursor(): void {
        do ++$this->cursor; while ($this->cursor < $this->limit && (ord($this->current[$this->cursor]) & 0xc0) == 0x80);
    }

    protected function dec_cursor(): void {
        do --$this->cursor; while ($this->cursor > $this->limit_backward && (ord($this->current[$this->cursor]) & 0xc0) == 0x80);
    }

    protected function hop(int $delta): bool {
        $res = $this->cursor;
        while ($delta > 0) {
            $delta--;
            if ($res >= $this->limit) {
                return false;
            }
            do {
                $res++;
            } while ($res < $this->limit && (ord($this->current[$res]) & 0xc0) == 0x80);
        }
        $this->cursor = $res;
        return true;
    }

    protected function hop_checked(int $delta): bool {
        return $delta >= 0 && $this->hop($delta);
    }

    protected function hop_back(int $delta): bool {
        $res = $this->cursor;
        while ($delta > 0) {
            $delta--;
            if ($res <= $this->limit_backward) {
                return false;
            }
            do {
                $res--;
            } while ($res > $this->limit_backward && (ord($this->current[$res]) & 0xc0) == 0x80);
        }
        $this->cursor = $res;
        return true;
    }

    protected function hop_back_checked(int $delta): bool {
        return $delta >= 0 && $this->hop_back($delta);
    }

    /**
     * Public entry point for stemming a word
     */
    public function stemWord(string $word): string {
        $this->current = $word;
        $this->cursor = 0;
        $this->limit = strlen($word);
        $this->limit_backward = 0;
        $this->bra = $this->cursor;
        $this->ket = $this->limit;
        $this->stem();
        return $this->current;
    }
}
