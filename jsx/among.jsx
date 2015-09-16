import "base-stemmer.jsx";

class Among
{
    var s_size : int;           /* search string */
    var s : string;             /* search string */
    var substring_i : int;      /* index to longest matching substring */
    var result : int;           /* result of the lookup */
    var instance : Nullable.<BaseStemmer> ;
                                /* object to invoke method on. It is a BaseStemmer */
    var method : Nullable.<(BaseStemmer) -> boolean>;
                                /* method to use if substring matches */

    function constructor (s : string, substring_i : int, result : int)
    {
        this.s_size = s.length;
        this.s = s;
        this.substring_i = substring_i;
	this.result = result;
        this.method = null;
	this.instance = null;
    }

    function constructor (s : string, substring_i : int, result : int,
                          method : (BaseStemmer) -> boolean, instance : BaseStemmer)
    {
        this.s_size = s.length;
        this.s = s;
        this.substring_i = substring_i;
	this.result = result;
        this.method = method;
	this.instance = instance;
    }
}
