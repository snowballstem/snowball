package snowball

import (
	"log"
	"strings"
	"unicode/utf8"
)

// Env represents the Snowball execution environment
type Env struct {
	current       string
	Cursor        uint
	Limit         uint
	LimitBackward uint
	Bra           uint
	Ket           uint
}

// NewEnv creates a new Snowball execution environment on the provided string
func NewEnv(val string) *Env {
	return &Env{
		current:       val,
		Cursor:        0,
		Limit:         uint(len(val)),
		LimitBackward: 0,
		Bra:           0,
		Ket:           uint(len(val)),
	}
}

func (env *Env) Current() string {
	return env.current
}

func (env *Env) SetCurrent(s string) {
	env.current = s
}

func (env *Env) ReplaceS(bra, ket uint, s string) int32 {
	adjustment := int32(len(s)) - (int32(ket) - int32(bra))

	result, _ := splitAt(env.current, bra)
	_, rhs := splitAt(env.current, ket)
	result += s
	result += rhs

	newLim := int32(env.Limit) + adjustment
	env.Limit = uint(newLim)

	if env.Cursor >= ket {
		newCur := int32(env.Cursor) + adjustment
		env.Cursor = uint(newCur)
	} else if env.Cursor > bra {
		env.Cursor = bra
	}

	env.current = result
	return adjustment
}

func (env *Env) EqS(s string) bool {
	if env.Cursor >= env.Limit {
		return false
	}

	if strings.HasPrefix(env.current[env.Cursor:], s) {
		env.Cursor += uint(len(s))
		for !onCharBoundary(env.current, env.Cursor) {
			env.Cursor++
		}
		return true
	}
	return false
}

func (env *Env) EqSB(s string) bool {
	if int32(env.Cursor)-int32(env.LimitBackward) < int32(len(s)) {
		return false
	} else if !onCharBoundary(env.current, env.Cursor-uint(len(s))) ||
		!strings.HasPrefix(env.current[env.Cursor-uint(len(s)):], s) {
		return false
	} else {
		env.Cursor -= uint(len(s))
		return true
	}
}

func (env *Env) SliceFrom(s string) bool {
	bra, ket := env.Bra, env.Ket
	env.ReplaceS(bra, ket, s)
	return true
}

func (env *Env) NextChar() {
	env.Cursor++
	for !onCharBoundary(env.current, env.Cursor) {
		env.Cursor++
	}
}

func (env *Env) PrevChar() {
	env.Cursor--
	for !onCharBoundary(env.current, env.Cursor) {
		env.Cursor--
	}
}

func (env *Env) ByteIndexForHop(delta int32) int32 {
	if delta > 0 {
		res := env.Cursor
		for delta > 0 {
			res++
			delta--
			for res <= uint(len(env.current)) && !onCharBoundary(env.current, res) {
				res++
			}
		}
		return int32(res)
	} else if delta < 0 {
		res := env.Cursor
		for delta < 0 {
			res--
			delta++
			for res >= 0 && !onCharBoundary(env.current, res) {
				res--
			}
		}
		return int32(res)
	} else {
		return int32(env.Cursor)
	}
}

func (env *Env) InGrouping(chars []byte, min, max int32) bool {
	if env.Cursor >= env.Limit {
		return false
	}

	r, _ := utf8.DecodeRuneInString(env.current[env.Cursor:])
	if r != utf8.RuneError {
		if r > max || r < min {
			return false
		}
		r -= min
		if (chars[uint(r>>3)] & (0x1 << uint(r&0x7))) == 0 {
			return false
		}
		env.NextChar()
		return true
	}
	return false
}

func (env *Env) InGroupingB(chars []byte, min, max int32) bool {
	if env.Cursor <= env.LimitBackward {
		return false
	}
	env.PrevChar()
	r, _ := utf8.DecodeRuneInString(env.current[env.Cursor:])
	if r != utf8.RuneError {
		env.NextChar()
		if r > max || r < min {
			return false
		}
		r -= min
		if (chars[uint(r>>3)] & (0x1 << uint(r&0x7))) == 0 {
			return false
		}
		env.PrevChar()
		return true
	}
	return false
}

func (env *Env) OutGrouping(chars []byte, min, max int32) bool {
	if env.Cursor >= env.Limit {
		return false
	}
	r, _ := utf8.DecodeRuneInString(env.current[env.Cursor:])
	if r != utf8.RuneError {
		if r > max || r < min {
			env.NextChar()
			return true
		}
		r -= min
		if (chars[uint(r>>3)] & (0x1 << uint(r&0x7))) == 0 {
			env.NextChar()
			return true
		}
	}
	return false
}

func (env *Env) OutGroupingB(chars []byte, min, max int32) bool {
	if env.Cursor <= env.LimitBackward {
		return false
	}
	env.PrevChar()
	r, _ := utf8.DecodeRuneInString(env.current[env.Cursor:])
	if r != utf8.RuneError {
		env.NextChar()
		if r > max || r < min {
			env.PrevChar()
			return true
		}
		r -= min
		if (chars[uint(r>>3)] & (0x1 << uint(r&0x7))) == 0 {
			env.PrevChar()
			return true
		}
	}
	return false
}

func (env *Env) SliceDel() bool {
	return env.SliceFrom("")
}

func (env *Env) Insert(bra, ket uint, s string) {
	adjustment := env.ReplaceS(bra, ket, s)
	if bra <= env.Bra {
		env.Bra = uint(int32(env.Bra) + adjustment)
	}
	if bra <= env.Ket {
		env.Ket = uint(int32(env.Ket) + adjustment)
	}
}

func (env *Env) SliceTo() string {
	return env.current[env.Bra:env.Ket]
}

func (env *Env) FindAmong(amongs []*Among, ctx interface{}) int32 {
	i := int32(0)
	j := int32(len(amongs))

	c := env.Cursor
	l := env.Limit

	commonI := uint(0)
	commonJ := uint(0)

	firstKeyInspected := false
	for {
		k := i + ((j - i) >> 1)
		diff := int32(0)
		common := min(commonI, commonJ)
		w := amongs[uint(k)]
		for lvar := common; lvar < uint(len(w.Str)); lvar++ {
			if c+common == l {
				diff--
				break
			}
			diff = int32(env.current[c+common]) - int32(w.Str[lvar])
			if diff != 0 {
				break
			}
			common++
		}
		if diff < 0 {
			j = k
			commonJ = common
		} else {
			i = k
			commonI = common
		}
		if j-i <= 1 {
			if i > 0 {
				break
			}
			if j == i {
				break
			}
			if firstKeyInspected {
				break
			}
			firstKeyInspected = true
		}
	}

	for {
		w := amongs[uint(i)]
		if commonI >= uint(len(w.Str)) {
			env.Cursor = c + uint(len(w.Str))
			if w.F != nil {
				res := w.F(env, ctx)
				env.Cursor = c + uint(len(w.Str))
				if res {
					return w.B
				}
			} else {
				return w.B
			}
		}
		i = w.A
		if i < 0 {
			return 0
		}
	}
}

func (env *Env) FindAmongB(amongs []*Among, ctx interface{}) int32 {
	i := int32(0)
	j := int32(len(amongs))

	c := env.Cursor
	lb := env.LimitBackward

	commonI := uint(0)
	commonJ := uint(0)

	firstKeyInspected := false

	for {
		k := i + ((j - i) >> 1)
		diff := int32(0)
		common := min(commonI, commonJ)
		w := amongs[uint(k)]
		for lvar := len(w.Str) - int(common) - 1; lvar >= 0; lvar-- {
			if c-common == lb {
				diff--
				break
			}
			diff = int32(env.current[c-common-1]) - int32(w.Str[lvar])
			if diff != 0 {
				break
			}
			// Count up commons. But not one character but the byte width of that char
			common++
		}
		if diff < 0 {
			j = k
			commonJ = common
		} else {
			i = k
			commonI = common
		}
		if j-i <= 1 {
			if i > 0 {
				break
			}
			if j == i {
				break
			}
			if firstKeyInspected {
				break
			}
			firstKeyInspected = true
		}
	}
	for {
		w := amongs[uint(i)]
		if commonI >= uint(len(w.Str)) {
			env.Cursor = c - uint(len(w.Str))
			if w.F != nil {
				res := w.F(env, ctx)
				env.Cursor = c - uint(len(w.Str))
				if res {
					return w.B
				}
			} else {
				return w.B
			}
		}
		i = w.A
		if i < 0 {
			return 0
		}
	}
}

func (env *Env) Debug(count, lineNumber int) {
	log.Printf("snowball debug, count: %d, line: %d", count, lineNumber)
}

func (env *Env) Clone() *Env {
	clone := *env
	return &clone
}
