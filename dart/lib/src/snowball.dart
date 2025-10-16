// ignore_for_file: non_constant_identifier_names, curly_braces_in_flow_control_structures

library;

/// Internal class used by Snowball stemmers
class Among {
  Among(this.s, this.substring_i, this.result, [this.function_index = 0]);

  /// search string
  final String s;

  /// index to longest matching substring
  final int substring_i;

  /// result of the lookup
  final int result;

  /// function index, 0 if none
  final int function_index;
}

class SnowballProgram {
  String current = '';

  int cursor = 0;
  int limit = 0;
  int limit_backward = 0;
  int bra = 0;
  int ket = 0;
  int af = 0;

  SnowballProgram();

  SnowballProgram.from(SnowballProgram other)
    : current = other.current,
      cursor = other.cursor,
      limit = other.limit,
      limit_backward = other.limit_backward,
      bra = other.bra,
      ket = other.ket,
      af = other.af;

  void init(String s) {
    current = s;
    cursor = 0;
    limit = current.length;
    limit_backward = 0;
    bra = cursor;
    ket = limit;
  }

  void copy_from(SnowballProgram other) {
    current = other.current;
    cursor = other.cursor;
    limit = other.limit;
    limit_backward = other.limit_backward;
    bra = other.bra;
    ket = other.ket;
  }

  bool in_grouping(String s, int min, int max) {
    if (cursor >= limit) return false;
    int ch = current.codeUnitAt(cursor);
    if (ch > max || ch < min) return false;
    ch -= min;
    if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) == 0) return false;
    cursor++;
    return true;
  }

  bool go_in_grouping(String s, int min, int max) {
    while (cursor < limit) {
      int ch = current.codeUnitAt(cursor);
      if (ch > max || ch < min) return true;
      ch -= min;
      if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) == 0) return true;
      cursor++;
    }
    return false;
  }

  bool in_grouping_b(String s, int min, int max) {
    if (cursor <= limit_backward) return false;
    int ch = current.codeUnitAt(cursor - 1);
    if (ch > max || ch < min) return false;
    ch -= min;
    if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) == 0) return false;
    cursor--;
    return true;
  }

  bool go_in_grouping_b(String s, int min, int max) {
    while (cursor > limit_backward) {
      int ch = current.codeUnitAt(cursor - 1);
      if (ch > max || ch < min) return true;
      ch -= min;
      if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) == 0) return true;
      cursor--;
    }
    return false;
  }

  bool out_grouping(String s, int min, int max) {
    if (cursor >= limit) return false;
    int ch = current.codeUnitAt(cursor);
    if (ch > max || ch < min) {
      cursor++;
      return true;
    }
    ch -= min;
    if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) == 0) {
      cursor++;
      return true;
    }
    return false;
  }

  bool go_out_grouping(String s, int min, int max) {
    while (cursor < limit) {
      int ch = current.codeUnitAt(cursor);
      if (ch <= max && ch >= min) {
        ch -= min;
        if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) != 0) {
          return true;
        }
      }
      cursor++;
    }
    return false;
  }

  bool out_grouping_b(String s, int min, int max) {
    if (cursor <= limit_backward) return false;
    int ch = current.codeUnitAt(cursor - 1);
    if (ch > max || ch < min) {
      cursor--;
      return true;
    }
    ch -= min;
    if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) == 0) {
      cursor--;
      return true;
    }
    return false;
  }

  bool go_out_grouping_b(String s, int min, int max) {
    while (cursor > limit_backward) {
      int ch = current.codeUnitAt(cursor - 1);
      if (ch <= max && ch >= min) {
        ch -= min;
        if ((s.codeUnitAt(ch >> 3) & (0X1 << (ch & 0X7))) != 0) {
          return true;
        }
      }
      cursor--;
    }
    return false;
  }

  bool eq_s(String s) {
    if (limit - cursor < s.length) return false;
    if (!current.startsWith(s, cursor)) return false;
    cursor += s.length;
    return true;
  }

  bool eq_s_b(String s) {
    if (cursor - limit_backward < s.length) return false;
    if (!current.substring(0, cursor).endsWith(s)) return false;
    cursor -= s.length;
    return true;
  }

  int find_among(List<Among> v, bool Function()? call_among_func) {
    int i = 0;
    int j = v.length;

    int c = cursor;
    int l = limit;

    int common_i = 0;
    int common_j = 0;

    bool first_key_inspected = false;

    while (true) {
      int k = i + ((j - i) >> 1);
      int diff = 0;
      int common = common_i < common_j ? common_i : common_j; // smaller
      Among w = v[k];
      for (int i2 = common; i2 < w.s.length; i2++) {
        if (c + common == l) {
          diff = -1;
          break;
        }
        diff = current.codeUnitAt(c + common) - w.s.codeUnitAt(i2);
        if (diff != 0) break;
        common++;
      }
      if (diff < 0) {
        j = k;
        common_j = common;
      } else {
        i = k;
        common_i = common;
      }
      if (j - i <= 1) {
        if (i > 0) break; // v->s has been inspected
        if (j == i) break; // only one item in v

        // - but now we need to go round once more to get
        // v->s inspected. This looks messy, but is actually
        // the optimal approach.

        if (first_key_inspected) break;
        first_key_inspected = true;
      }
    }
    while (true) {
      Among w = v[i];
      if (common_i >= w.s.length) {
        cursor = c + w.s.length;
        if (w.function_index == 0) return w.result;
        af = w.function_index;
        if (call_among_func?.call() ?? false) {
          cursor = c + w.s.length;
          return w.result;
        }
      }
      i = w.substring_i;
      if (i < 0) return 0;
    }
  }

  int find_among_b(List<Among> v, bool Function()? call_among_func) {
    int i = 0;
    int j = v.length;

    int c = cursor;
    int lb = limit_backward;

    int common_i = 0;
    int common_j = 0;

    bool first_key_inspected = false;

    while (true) {
      int k = i + ((j - i) >> 1);
      int diff = 0;
      int common = common_i < common_j ? common_i : common_j;
      Among w = v[k];
      for (int i2 = w.s.length - 1 - common; i2 >= 0; i2--) {
        if (c - common == lb) {
          diff = -1;
          break;
        }
        diff = current.codeUnitAt(c - 1 - common) - w.s.codeUnitAt(i2);
        if (diff != 0) break;
        common++;
      }
      if (diff < 0) {
        j = k;
        common_j = common;
      } else {
        i = k;
        common_i = common;
      }
      if (j - i <= 1) {
        if (i > 0) break;
        if (j == i) break;
        if (first_key_inspected) break;
        first_key_inspected = true;
      }
    }
    while (true) {
      Among w = v[i];
      if (common_i >= w.s.length) {
        cursor = c - w.s.length;
        if (w.function_index == 0) return w.result;
        af = w.function_index;
        if (call_among_func?.call() ?? false) {
          cursor = c - w.s.length;
          return w.result;
        }
      }
      i = w.substring_i;
      if (i < 0) return 0;
    }
  }

  int replace_s(int c_bra, int c_ket, String s) {
    final adjustment = s.length - (c_ket - c_bra);
    current = current.replaceRange(c_bra, c_ket, s);
    limit += adjustment;
    if (cursor >= c_ket) {
      cursor += adjustment;
    } else if (cursor > c_bra) {
      cursor = c_bra;
    }
    return adjustment;
  }

  void slice_check() {
    assert(bra >= 0, 'bra=$bra');
    assert(bra <= ket, 'bra=$bra,ket=$ket');
    assert(ket <= limit, 'ket=$ket,limit=$limit');
    assert(limit <= current.length, 'limit=$limit,length=${current.length}');
  }

  void slice_from(String s) {
    slice_check();
    replace_s(bra, ket, s);
    ket = bra + s.length;
  }

  void slice_del() {
    slice_from("");
  }

  void insert(int c_bra, int c_ket, String s) {
    int adjustment = replace_s(c_bra, c_ket, s);
    if (c_bra <= bra) bra += adjustment;
    if (c_bra <= ket) ket += adjustment;
  }

  String slice_to() {
    slice_check();
    return current.substring(bra, ket);
  }

  String assign_to() {
    return current.substring(0, limit);
  }
}

abstract class SnowballStemmer extends SnowballProgram {
  bool stem();
}
