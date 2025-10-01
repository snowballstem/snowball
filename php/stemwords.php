<?php

$lang = $argv[1] ?? 'english';
$parent = realpath(__DIR__.'/..');
$phpfile = $parent.'/php_out/'.$lang.'-stemmer.php';
if (!file_exists($phpfile)) {
    fwrite(STDERR, "PHP stemmer not found at $phpfile\n");
    exit(1);
}

require __DIR__.'/base-stemmer.php';
require $phpfile;

$class = 'Snowball'.implode('', array_map('ucfirst', explode('_', $lang))).'Stemmer';
if (!class_exists($class)) {
    fwrite(STDERR, "$class not included from $phpfile\n");
    exit(1);
}
$stemmer = new $class;

while ($word = fgets(STDIN)) {
    $word = strtolower(rtrim($word, "\n"));
    $stem = $stemmer->stemWord($word);
    echo $stem, "\n";
}
