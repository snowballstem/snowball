import 'among.dart';

class BaseStemmer {
  late String current;
  late int cursor;
  late int limit;
  late int limit_backward;
  late int bra;
  late int ket;

  BaseStemmer() {
    current = '';
    init();
  }

  void init() {
    cursor = 0;
    limit = current.length;
    limit_backward = 0;
    bra = cursor;
    ket = limit;
  }

  void setCurrent(String value) {
    current = value;
    init();
  }

  String getCurrent() {
    return current.toString();
  }

  void copy_from(BaseStemmer other) {
    current = other.current;
    cursor = other.cursor;
    limit = other.limit;
    limit_backward = other.limit_backward;
    bra = other.bra;
    ket = other.ket;
  }

  bool in_grouping(List<int> s, int min, int max) {
    if (cursor >= limit) {
      return false;
    }
    var ch = current.codeUnitAt(cursor);
    if ((ch > max) || (ch < min)) {
      return false;
    }
    ch -= min;
    if ((s[ch >> 3] & (1 << (ch & 7))) == 0) {
      return false;
    }
    cursor++;
    return true;
  }

  bool in_grouping_b(List<int> s, int min, int max) {
    if (cursor <= limit_backward) {
      return false;
    }
    var ch = current.codeUnitAt(cursor - 1);
    if ((ch > max) || (ch < min)) {
      return false;
    }
    ch -= min;
    if ((s[ch >> 3] & (1 << (ch & 7))) == 0) {
      return false;
    }
    cursor--;
    return true;
  }

  bool out_grouping(List<int> s, int min, int max) {
    if (cursor >= limit) {
      return false;
    }
    var ch = current.codeUnitAt(cursor);
    if ((ch > max) || (ch < min)) {
      cursor++;
      return true;
    }
    ch -= min;
    if ((s[ch >> 3] & (1 << (ch & 7))) == 0) {
      cursor++;
      return true;
    }
    return false;
  }

  bool out_grouping_b(List<int> s, int min, int max) {
    if (cursor <= limit_backward) {
      return false;
    }
    var ch = current.codeUnitAt(cursor - 1);
    if ((ch > max) || (ch < min)) {
      cursor--;
      return true;
    }
    ch -= min;
    if ((s[ch >> 3] & (1 << (ch & 7))) == 0) {
      cursor--;
      return true;
    }
    return false;
  }

  bool eq_s(String s) {
    if ((limit - cursor) < s.length) {
      return false;
    }
    int i;
    for ((i = 0); i != s.length; i++) {
      if (current.codeUnitAt(cursor + i) != s.codeUnitAt(i)) {
        return false;
      }
    }
    cursor += s.length;
    return true;
  }

  bool eq_s_b(String s) {
    if ((cursor - limit_backward) < s.length) {
      return false;
    }
    int i;
    for ((i = 0); i != s.length; i++) {
      if (current.codeUnitAt((cursor - s.length) + i) != s.codeUnitAt(i)) {
        return false;
      }
    }
    cursor -= s.length;
    return true;
  }

  int find_among(List<Among> v) {
    var i = 0;
    var j = v.length;
    var c = cursor;
    var l = limit;
    var common_i = 0;
    var common_j = 0;
    var first_key_inspected = false;
    while (true) {
      var k = (i + ((j - i) >> 1));
      var diff = 0;
      var common = ((common_i < common_j) ? common_i : common_j);
      var w = v[k];
      int i2;
      for ((i2 = common); i2 < w.s.length; i2++) {
        if ((c + common) == l) {
          diff = (-1);
          break;
        }
        diff = (current.codeUnitAt(c + common) - w.s.codeUnitAt(i2));
        if (diff != 0) {
          break;
        }
        common++;
      }
      if (diff < 0) {
        j = k;
        common_j = common;
      } else {
        i = k;
        common_i = common;
      }
      if ((j - i) <= 1) {
        if (i > 0) {
          break;
        }
        if (j == i) {
          break;
        }
        if (first_key_inspected) {
          break;
        }
        first_key_inspected = true;
      }
    }
    while (true) {
      var w = v[i];
      if (common_i >= w.s.length) {
        cursor = (c + w.s.length);
        if (w.method == null) {
          return w.result;
        }
        var res = w.method!();
        cursor = (c + w.s.length);
        if (res) {
          return w.result;
        }
      }
      i = w.substring_i;
      if (i < 0) {
        return 0;
      }
    }
  }

  int find_among_b(List<Among> v) {
    var i = 0;
    var j = v.length;
    var c = cursor;
    var lb = limit_backward;
    var common_i = 0;
    var common_j = 0;
    var first_key_inspected = false;
    while (true) {
      var k = (i + ((j - i) >> 1));
      var diff = 0;
      var common = ((common_i < common_j) ? common_i : common_j);
      var w = v[k];
      int i2;
      for ((i2 = ((w.s.length - 1) - common)); i2 >= 0; i2--) {
        if ((c - common) == lb) {
          diff = (-1);
          break;
        }
        diff = (current.codeUnitAt((c - 1) - common) - w.s.codeUnitAt(i2));
        if (diff != 0) {
          break;
        }
        common++;
      }
      if (diff < 0) {
        j = k;
        common_j = common;
      } else {
        i = k;
        common_i = common;
      }
      if ((j - i) <= 1) {
        if (i > 0) {
          break;
        }
        if (j == i) {
          break;
        }
        if (first_key_inspected) {
          break;
        }
        first_key_inspected = true;
      }
    }
    while (true) {
      var w = v[i];
      if (common_i >= w.s.length) {
        cursor = (c - w.s.length);
        if (w.method == null) {
          return w.result;
        }
        var res = w.method!();
        cursor = (c - w.s.length);
        if (res) {
          return w.result;
        }
      }
      i = w.substring_i;
      if (i < 0) {
        return 0;
      }
    }
  }

  int replace_s(int c_bra, int c_ket, String s) {
    var adjustment = (s.length - (c_ket - c_bra));
    current = current.replaceRange(c_bra, c_ket, s);
    limit += adjustment;
    if (cursor >= c_ket) {
      cursor += adjustment;
    } else {
      if (cursor > c_bra) {
        cursor = c_bra;
      }
    }
    return adjustment;
  }

  void slice_check() {
    if ((((bra < 0) || (bra > ket)) || (ket > limit)) ||
        (limit > current.length)) {
      print('ERROR: faulty slice operation');
    }
  }

  void slice_from(String s) {
    slice_check();
    replace_s(bra, ket, s);
  }

  void slice_del() {
    slice_from('');
  }

  void insert(int c_bra, int c_ket, String s) {
    var adjustment = replace_s(c_bra, c_ket, s);
    if (c_bra <= bra) {
      bra += adjustment;
    }
    if (c_bra <= ket) {
      ket += adjustment;
    }
  }

  String slice_to(String s) {
    slice_check();
    return s.replaceRange(0, s.length, current.substring(bra, ket));
  }

  String assign_to(String s) {
    return s.replaceRange(0, s.length, current.substring(0, limit));
  }
}
