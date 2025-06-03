<?php
/**
 * Manually converted from JavaScript:
 * https://github.com/snowballstem/snowball/blob/master/javascript/base-stemmer.js
 * 
 * Build a stemmer from Snowball, as follows:
 * ~$ ./snowball algorithms/english.sbl -php -o php/english-stemmer
 */
abstract class SnowballStemmer {

    protected string $current = '';
    protected int $cursor = 0;
    protected int $limit = 0;
    protected int $limit_backward = 0;
    protected int $bra = 0;
    protected int $ket = 0;
    
    
    abstract public function stem():bool;

    
    protected function setCurrent( string $value ):void {
        $this->current = $value;
        $this->cursor = 0;
        $this->limit = $this->currentLength();
        $this->limit_backward = 0;
        $this->bra = $this->cursor;
        $this->ket = $this->limit;
    }


    protected function getCurrent():string {
        return $this->current;
    }


    /**
     * @param int[] $s
     */
    protected function in_grouping( array $s, int $min, int $max ):bool {
        if( $this->cursor >= $this->limit ){
            return false;
        }
        $ch = $this->currentCharCodeAt($this->cursor);
        if ($ch > $max || $ch < $min){
            return false;
        }
        $ch -= $min;
        if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) === 0 ){
            return false;
        }
        $this->cursor++;
        return true;
    }


    /**
     * @param int[] $s
     */
    protected function go_in_grouping( array $s, int $min, int $max ):bool {
        while ( $this->cursor < $this->limit) {
            $ch = $this->currentCharCodeAt($this->cursor);
            if( $ch > $max || $ch < $min ){
                return true;
            }
            $ch -= $min;
            if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) === 0 ){
                return true;
            }
            $this->cursor++;
        }
        return false;
    }



    /**
     * @param int[] $s
     */
    protected function in_grouping_b( array $s, int $min, int $max ):bool {
        if( $this->cursor <= $this->limit_backward ){
            return false;
        }
        $ch = $this->currentCharCodeAt($this->cursor - 1);
        if ($ch > $max || $ch < $min){
            return false;
        }
        $ch -= $min;
        if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) === 0 ){
            return false;
        }
        $this->cursor--;
        return true;
    }


    /**
     * @param int[] $s
     */
    protected function go_in_grouping_b( array $s, int $min, int $max ):bool {
        while ( $this->cursor > $this->limit_backward) {
            $ch = $this->currentCharCodeAt($this->cursor - 1);
            if ($ch > $max || $ch < $min){ 
                return true;
            }
            $ch -= $min;
            if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) === 0){
                return true;
            }
            $this->cursor--;
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function out_grouping( array $s, int $min, int $max ):bool {
        if ($this->cursor >= $this->limit){
            return false;
        }
        $ch = $this->currentCharCodeAt($this->cursor);
        if ($ch > $max || $ch < $min) {
            $this->cursor++;
            return true;
        }
        $ch -= $min;
        if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) === 0) {
            $this->cursor++;
            return true;
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function go_out_grouping ( array $s, int $min, int $max ):bool {
        while ( $this->cursor < $this->limit) {
            $ch = $this->currentCharCodeAt($this->cursor);
            if ($ch <= $max && $ch >= $min) {
                $ch -= $min;
                if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) !== 0 ) {
                    return true;
                }
            }
            $this->cursor++;
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function out_grouping_b( array $s, int $min, int $max ):bool {
        if ($this->cursor <= $this->limit_backward){
            return false;
        }
        $ch = $this->currentCharCodeAt($this->cursor - 1);
        if ($ch > $max || $ch < $min) {
            $this->cursor--;
            return true;
        }
        $ch -= $min;
        if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) === 0 ) {
            $this->cursor--;
            return true;
        }
        return false;
    }


    /**
     * @param int[] $s
     */
    protected function go_out_grouping_b ( array $s, int $min, int $max ):bool {
        while ( $this->cursor > $this->limit_backward) {
            $ch = $this->currentCharCodeAt($this->cursor - 1);
            if ($ch <= $max && $ch >= $min) {
                $ch -= $min;
                if( ($s[$ch >> 3] & (0x1 << ($ch & 0x7))) !== 0) {
                    return true;
                }
            }
            $this->cursor--;
        }
        return false;
    }


    protected function eq_s( string $s ):bool {
        $slength = self::lengthOf($s);
        if ($this->limit - $this->cursor < $slength ){
            return false;
        }
        if ( $this->currentSlice($this->cursor, $this->cursor + $slength) !== $s) {
            return false;
        }
        $this->cursor += $slength;
        return true;
    }


    protected function eq_s_b( string $s ):bool {
        $slength = self::lengthOf($s);
        if ($this->cursor - $this->limit_backward < $slength){
            return false;
        }
        if ($this->currentSlice($this->cursor - $slength, $this->cursor) !== $s){
            return false;
        }
        $this->cursor -= $slength;
        return true;
    }


    /**
     * @param array[] $v
     */
    protected function find_among( array $v ):int {
        $i = 0;
        $j = count($v);

        $c = $this->cursor;
        $l = $this->limit;

        $common_i = 0;
        $common_j = 0;

        $first_key_inspected = false;

        while( true ){
            $k = $i + ( ($j-$i) >> 1);
            $diff = 0;
            $common = min($common_i, $common_j); // smaller
            // w[0]: string, w[1]: substring_i, w[2]: result, w[3]: function (optional)
            $w = $v[$k];
            $w0length = self::lengthOf($w[0]);
            for( $i2 = $common; $i2 < $w0length; $i2++ ){
                if ( $c + $common === $l){
                    $diff = -1;
                    break;
                }
                $diff = $this->currentCharCodeAt($c+$common) - self::charCodeAt($w[0],$i2);
                if ($diff !== 0){
                    break;
                }
                $common++;
            }
            if ($diff < 0){
                $j = $k;
                $common_j = $common;
            }
            else {
                $i = $k;
                $common_i = $common;
            }
            if ($j - $i <= 1) {
                if ($i > 0){
                    break;
                } 
                // v->s has been inspected
                if ( $j === $i ){
                    break;
                } 
                // only one item in v

                // - but now we need to go round once more to get
                // v->s inspected. This looks messy, but is actually
                // the optimal approach.

                if ( $first_key_inspected ){
                    break;
                }
                $first_key_inspected = true;
            }
        }
        do {
            $w = $v[$i];
            $w0length = self::lengthOf($w[0]);
            if( $common_i >= $w0length ) {
                $this->cursor = $c + $w0length;
                if ( count($w) < 4){
                    return $w[2];
                }
                $res = call_user_func( [ $this, $w[3] ] );
                $this->cursor = $c + $w0length;
                if( $res ){
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
    protected function find_among_b( array $v ):int {
        $i = 0;
        $j = count($v);

        $c = $this->cursor;
        $lb = $this->limit_backward;

        $common_i = 0;
        $common_j = 0;

        $first_key_inspected = false;

        while( true ) {
            $k = $i + ( ($j-$i) >> 1);
            $diff = 0;
            $common = min($common_i, $common_j);
            $w = $v[$k];
            $w0length = self::lengthOf($w[0]);
            for ( $i2 = $w0length - 1 - $common; $i2 >= 0; $i2-- ){
                if ($c - $common == $lb){
                    $diff = -1;
                    break;
                }
                $diff = $this->currentCharCodeAt($c - 1 - $common) - self::charCodeAt($w[0],$i2);
                if ($diff != 0){
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
                if ( $i > 0 || $j === $i || $first_key_inspected ){
                    break;
                }
                $first_key_inspected = true;
            }
        }
        do {
            $w = $v[$i];
            $w0length = self::lengthOf($w[0]);
            if ( $common_i >= $w0length ){
                $this->cursor = $c - $w0length;
                if ( count($w) < 4){
                    return $w[2];
                }
                $res = call_user_func( [ $this, $w[3] ] );
                $this->cursor = $c - $w0length;
                if ($res){
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
    private function replace_s( int $c_bra, int $c_ket, string $s ):int {
        $slength = self::lengthOf($s);
        $adjustment = $slength - ($c_ket - $c_bra);
        $this->current = $this->currentSlice(0,$c_bra) . $s . $this->currentSlice($c_ket);
        $this->limit += $adjustment;
        if ( $this->cursor >= $c_ket){
            $this->cursor += $adjustment;
        }
        else if ( $this->cursor > $c_bra){
            $this->cursor = $c_bra;
        }
        return $adjustment;
    }


    private function slice_check():void {
        if( $this->bra < 0 ||
            $this->bra > $this->ket ||
            $this->ket > $this->limit ||
            $this->limit > $this->currentLength()
        ){
            throw new LogicException('Faulty slice operation');
        }
    }


    protected function slice_from( string $s ):void {
        $this->slice_check();
        $this->replace_s($this->bra, $this->ket, $s );
    }


    protected function slice_del():void {
        $this->slice_from('');
    }


    protected function insert( int $c_bra, int $c_ket, string $s ):void {
        $adjustment = $this->replace_s($c_bra, $c_ket, $s);
        $c_bra <= $this->bra and $this->bra += $adjustment;
        $c_bra <= $this->ket and $this->ket += $adjustment;
    }


    protected function slice_to():string {
        $this->slice_check();
        return $this->currentSlice($this->bra, $this->ket);
    }


    protected function assign_to():string {
        return $this->currentSlice(0, $this->limit );
    }
    
    
    
    // -- Getters named similarly to JavaScript method. e.g. $this->current.length => $this->currentLength()

    
    private function currentCharCodeAt( int $offset ):int {
        return self::charCodeAt( $this->current, $offset );
    }
    
    protected function currentLength():int {
        return self::lengthOf( $this->current );
    }
    

    private function currentSlice( int $start, ?int $end = null ):string {
        return self::strSlice($this->current, $start, $end );
    }


    // Everything above here was "translated" blindly from JavaScript.
    // Below are some utilities to ensure PHP behaviour matches JavaScript for string manipulation etc...


    /**
     * As per String.prototype.length
     */
    public static function lengthOf( string $s ):int {
        return mb_strlen($s,'UTF-8');
    }


    /**
     * As per String.prototype.charCodeAt
     * @throws RangeException
     */
    public static function charCodeAt( string $s, int $offset ):int {
        $c = mb_substr($s,$offset,1,'UTF-8');
        if( '' === $c ){
            throw new RangeException('Bad character offset');
        }
        return IntlChar::ord($c);
    }


    /**
     * As per String.prototype.slice
     */
    public static function strSlice( string $s, int $start, ?int $end = null ):string {
        if( is_null($end) ){
            return mb_substr($s,$start,null,'UTF-8');
        }
        if( $start < 0 ){
            $start = max(0, self::lengthOf($s)+$start );
        }
        if( $end < 0 ){
            $end = max(0, self::lengthOf($s)+$end );
        }
        if( $end < $start ){
            return '';
        }
        return mb_substr( $s, $start, $end-$start, 'UTF-8');
    }
    

    /**
     * Public entry point for stemming a word
     */
    public function stemWord( string $word ):string {
        $this->setCurrent($word);
        $this->stem();
        return $this->getCurrent();
    }
    
    
}
