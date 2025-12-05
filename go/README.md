# Go Target for Snowball

The initial implementation was built as a port of the Rust target.  The initial focus has been on getting it to function, and making it work correctly.  No attempt has been made to beautify the implementation, generated code, or address performance issues.

## Usage

To generate Go source for a Snowball algorithm:
```
$ snowball path/to/algorithm.sbl -go -o algorithm
```

### Go specific options

`-Package`/`-P`: the package name used in the generated go file (defaults to `snowball`)

`-goruntime`/`-gor`: the import path used for the Go Snowball runtime (defaults to `github.com/snowballstem/snowball/go`)

Snowball 3.0.1 and earlier supported `-go`/`-gopackage`, but this was just an
alias for `-Package`/`-P` since Snowball 2.0.0.

## Code Organization

`compiler/generator_go.c` has the Go code generation logic

`go/` contains the default Go Snowball runtime support

`go/stemwords` contains the source for a Go version of the stemwords utility

`go/algorithms` location where the makefile generated code will end up

## Using the Generated Stemmers

Assuming you generated a stemmer, put that code in a package which is imported by this code as `english`.

```
env := snowball.NewEnv("")
env.SetCurrent("beautiful")
english.Stem(env)
fmt.Printf("stemmed word is: %s", env.Current())
```

If you are stemming many words you should reuse `env` as shown above.  If you
are stemming a single word, you can set the current string when you create it
with `env:= snowball.NewEnv("beautiful")`, but doing this when stemming many
words is not recommended as the overhead is measurable (stemwords for the arabic
stemmer on the sample vocabulary is about 12% faster if you reuse `env`).

## Testing

Only the existing Snowball algorithms have been used for testing.  This does not exercise all features of the language.

Run:

```
$ make check_go
```

An initial pass of fuzz-testing has been performed on the generated stemmers for the algorithms in this repo.  Each ran for 5 minutes and used an initial corpus seeded with 10k words from the algorithm's snowballstem-data voc.txt file.

## Known Limitations

- Code going through generate_dollar production has not been tested
- Code going through generate_debug production has not been tested
