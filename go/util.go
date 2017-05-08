package snowball

import "unicode/utf8"

func splitAt(str string, mid int) (string, string) {
	return str[:mid], str[mid:]
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}

func onCharBoundary(s string, pos int) bool {
	if pos >= len(s) {
		return true
	}
	return utf8.RuneStart(s[pos])
}

// RuneCountInString is a wrapper around utf8.RuneCountInString
// this allows us to not have to conditionally include
// the utf8 package into some stemmers and not others
func RuneCountInString(str string) int {
	return utf8.RuneCountInString(str)
}
