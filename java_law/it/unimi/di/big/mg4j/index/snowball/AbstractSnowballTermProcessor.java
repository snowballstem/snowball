package it.unimi.di.big.mg4j.index.snowball;

//import java.lang.reflect.InvocationTargetException;
import it.unimi.dsi.lang.MutableString;
import it.unimi.di.big.mg4j.index.TermProcessor;

public abstract class AbstractSnowballTermProcessor implements TermProcessor, Cloneable {

	/** Default generated serialVersionUID for Serializables */
	private static final long serialVersionUID = 1L;

	protected abstract boolean stem();

	public boolean processTerm( MutableString term ) {
		current = term;
		cursor = 0;
		limit = current.length();
		array = current.array();
		limit_backward = 0;
		bra = cursor;
		ket = limit;
		boolean b = stem();
		current = null;
		array = null;
		return b;
	}

	public boolean processPrefix( MutableString prefix ) {
		return prefix != null;
	}

	public AbstractSnowballTermProcessor copy() {
		try {
			return this.getClass().newInstance();
		}
		catch( Exception e ) {
			throw new RuntimeException( e );
		}
	}

	// current string
	protected MutableString current;

	protected char[] array;

	protected int cursor;

	protected int limit;

	protected int limit_backward;

	protected int bra;

	protected int ket;

	protected void copy_from( AbstractSnowballTermProcessor other ) {
		current = other.current;
		cursor = other.cursor;
		limit = other.limit;
		limit_backward = other.limit_backward;
		bra = other.bra;
		ket = other.ket;
	}

	protected boolean in_grouping( char[] s, int min, int max ) {
		if ( cursor >= limit ) return false;
		char ch = array[ cursor ];
		if ( ch > max || ch < min ) return false;
		ch -= min;
		if ( ( s[ ch >> 3 ] & ( 0X1 << ( ch & 0X7 ) ) ) == 0 ) return false;
		cursor++;
		return true;
	}

	protected boolean in_grouping_b( char[] s, int min, int max ) {
		if ( cursor <= limit_backward ) return false;
		char ch = array[ cursor - 1 ];
		if ( ch > max || ch < min ) return false;
		ch -= min;
		if ( ( s[ ch >> 3 ] & ( 0X1 << ( ch & 0X7 ) ) ) == 0 ) return false;
		cursor--;
		return true;
	}

	protected boolean out_grouping( char[] s, int min, int max ) {
		if ( cursor >= limit ) return false;
		char ch = array[ cursor ];
		if ( ch > max || ch < min ) {
			cursor++;
			return true;
		}
		ch -= min;
		if ( ( s[ ch >> 3 ] & ( 0X1 << ( ch & 0X7 ) ) ) == 0 ) {
			cursor++;
			return true;
		}
		return false;
	}

	protected boolean out_grouping_b( char[] s, int min, int max ) {
		if ( cursor <= limit_backward ) return false;
		char ch = array[ cursor - 1 ];
		if ( ch > max || ch < min ) {
			cursor--;
			return true;
		}
		ch -= min;
		if ( ( s[ ch >> 3 ] & ( 0X1 << ( ch & 0X7 ) ) ) == 0 ) {
			cursor--;
			return true;
		}
		return false;
	}

	protected boolean in_range( int min, int max ) {
		if ( cursor >= limit ) return false;
		char ch = array[ cursor ];
		if ( ch > max || ch < min ) return false;
		cursor++;
		return true;
	}

	protected boolean in_range_b( int min, int max ) {
		if ( cursor <= limit_backward ) return false;
		char ch = array[ cursor - 1 ];
		if ( ch > max || ch < min ) return false;
		cursor--;
		return true;
	}

	protected boolean out_range( int min, int max ) {
		if ( cursor >= limit ) return false;
		char ch = array[ cursor ];
		if ( !( ch > max || ch < min ) ) return false;
		cursor++;
		return true;
	}

	protected boolean out_range_b( int min, int max ) {
		if ( cursor <= limit_backward ) return false;
		char ch = array[ cursor - 1 ];
		if ( !( ch > max || ch < min ) ) return false;
		cursor--;
		return true;
	}

	protected boolean eq_s( int s_size, String s ) {
		if ( limit - cursor < s_size ) return false;
		int i;
		for ( i = 0; i != s_size; i++ ) {
			if ( array[ cursor + i ] != s.charAt( i ) ) return false;
		}
		cursor += s_size;
		return true;
	}

	protected boolean eq_s_b( int s_size, String s ) {
		if ( cursor - limit_backward < s_size ) return false;
		int i;
		for ( i = 0; i != s_size; i++ ) {
			if ( array[ cursor - s_size + i ] != s.charAt( i ) ) return false;
		}
		cursor -= s_size;
		return true;
	}

	protected boolean eq_v( MutableString s ) {
		return eq_s( s.length(), s.toString() );
	}

	protected boolean eq_v_b( MutableString s ) {
		return eq_s_b( s.length(), s.toString() );
	}

	protected int find_among( Among v[], BooleanStrategy strategies[], int v_size ) {
		int i = 0;
		int j = v_size;

		int c = cursor;
		int l = limit;

		int common_i = 0;
		int common_j = 0;

		boolean first_key_inspected = false;

		while ( true ) {
			int k = i + ( ( j - i ) >> 1 );
			int diff = 0;
			int common = common_i < common_j ? common_i : common_j; // smaller
			Among w = v[ k ];
			int i2;
			for ( i2 = common; i2 < w.s_size; i2++ ) {
				if ( c + common == l ) {
					diff = -1;
					break;
				}
				diff = array[ c + common ] - w.s[ i2 ];
				if ( diff != 0 ) break;
				common++;
			}
			if ( diff < 0 ) {
				j = k;
				common_j = common;
			}
			else {
				i = k;
				common_i = common;
			}
			if ( j - i <= 1 ) {
				if ( i > 0 ) break; // v->s has been inspected
				if ( j == i ) break; // only one item in v

				// - but now we need to go round once more to get
				// v->s inspected. This looks messy, but is actually
				// the optimal approach.

				if ( first_key_inspected ) break;
				first_key_inspected = true;
			}
		}
		while ( true ) {
			Among w = v[ i ];
			BooleanStrategy strategy = strategies[ i ];
			if ( common_i >= w.s_size ) {
				cursor = c + w.s_size;

				if( strategy == null ) return w.result;

				boolean res = strategy.invoke();

				cursor = c + w.s_size;
				if ( res ) return w.result;
			}
			i = w.substring_i;
			if ( i < 0 ) return 0;
		}
	}

	// find_among_b is for backwards processing. Same comments apply
	protected int find_among_b( Among v[], BooleanStrategy strategies[], int v_size ) {
		int i = 0;
		int j = v_size;

		int c = cursor;
		int lb = limit_backward;

		int common_i = 0;
		int common_j = 0;

		boolean first_key_inspected = false;

		while ( true ) {
			int k = i + ( ( j - i ) >> 1 );
			int diff = 0;
			int common = common_i < common_j ? common_i : common_j;
			Among w = v[ k ];
			int i2;
			for ( i2 = w.s_size - 1 - common; i2 >= 0; i2-- ) {
				if ( c - common == lb ) {
					diff = -1;
					break;
				}
				diff = array[ c - 1 - common ] - w.s[ i2 ];
				if ( diff != 0 ) break;
				common++;
			}
			if ( diff < 0 ) {
				j = k;
				common_j = common;
			}
			else {
				i = k;
				common_i = common;
			}
			if ( j - i <= 1 ) {
				if ( i > 0 ) break;
				if ( j == i ) break;
				if ( first_key_inspected ) break;
				first_key_inspected = true;
			}
		}
		while ( true ) {
			Among w = v[ i ];
			BooleanStrategy strategy = strategies[ i ];
			if ( common_i >= w.s_size ) {
				cursor = c - w.s_size;

				if ( strategy == null ) return w.result;

				boolean res = strategy.invoke();

				cursor = c - w.s_size;
				if ( res ) return w.result;
			}
			i = w.substring_i;
			if ( i < 0 ) return 0;
		}
	}

	/*
	 * to replace chars between c_bra and c_ket in current by the chars in s.
	 */
	protected int replace_s( int c_bra, int c_ket, String s ) {
		int adjustment = s.length() - ( c_ket - c_bra );
		current.replace( c_bra, c_ket, s );
		array = current.array();
		limit += adjustment;
		if ( cursor >= c_ket ) cursor += adjustment;
		else if ( cursor > c_bra ) cursor = c_bra;
		return adjustment;
	}

	protected void slice_check() {
		if ( bra < 0 || bra > ket || ket > limit || limit > current.length() ) {
			// this line could be removed
			System.err.println( "faulty slice operation" );
			/*  FIXME: report error somehow.
			 * fprintf(stderr, "faulty slice operation:\n"); debug(z, -1, 0); exit(1);
			 */
		}
	}

	protected void slice_from( String s ) {
		slice_check();
		replace_s( bra, ket, s );
	}

	protected void slice_del() {
		slice_from( "" );
	}

	protected void insert( int c_bra, int c_ket, String s ) {
		int adjustment = replace_s( c_bra, c_ket, s );
		if ( c_bra <= bra ) bra += adjustment;
		if ( c_bra <= ket ) ket += adjustment;
	}

	protected void insert( int c_bra, int c_ket, MutableString s ) {
		insert( c_bra, c_ket, s.toString() );
	}

	/* Copy the slice into the supplied MutableString */
	protected MutableString slice_to( MutableString s ) {
		slice_check();
		s.replace( 0, s.length(), current.substring( bra, ket ) );
		return s;
	}

	protected MutableString assign_to( MutableString s ) {
		s.replace( 0, s.length(), current.substring( 0, limit ) );
		return s;
	}

	/*
	 * extern void debug(struct SN_env * z, int number, int line_count) { int i; int limit =
	 * SIZE(z->p); //if (number >= 0) printf("%3d (line %4d): '", number, line_count); if (number >=
	 * 0) printf("%3d (line %4d): [%d]'", number, line_count,limit); for (i = 0; i <= limit; i++) {
	 * if (z->lb == i) printf("{"); if (z->bra == i) printf("["); if (z->c == i) printf("|"); if
	 * (z->ket == i) printf("]"); if (z->l == i) printf("}"); if (i < limit) { int ch = z->p[i]; if
	 * (ch == 0) ch = '#'; printf("%c", ch); } } printf("'\n"); }
	 */

};
