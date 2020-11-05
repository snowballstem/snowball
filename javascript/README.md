To generate a javascript stemmer for use with `node ./stemwords.js` in this
directory, run the following:

```bash
./snowball -p "(require('./base-stemmer'))" ./algorithms/english.sbl -js -o javascript/english-stemmer
```

Replace "english" with the target language as needed.
