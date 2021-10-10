typedef Method = bool Function();

class Among {
  final String s;
  final int substring_i;
  final int result;
  final Method? method;

  Among(this.s, this.substring_i, this.result, {this.method});
}
