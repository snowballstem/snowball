# Stemmers testing

For checking the correctness of the stemmer you can run some language test cases, for example:
```sh
bin/utf_to_sbl ./explanations/ukrainian.sbl.utf > algorithms/ukrainian.sbl && make

ruby ./tests/algorithms/ukrainian_test.rb
```
