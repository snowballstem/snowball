<?php
/*
 * For testing vocab files from snowball-data
 * run `make check_php`
 */

$lang = $argv[1] ?? 'english';
$parent = realpath(__DIR__.'/..');
$phpfile = $parent.'/php_out/'.$lang.'-stemmer.php';
if( ! file_exists($phpfile) ){
    fwrite(STDERR, "PHP stemmer not found at $phpfile\n");
    exit(1);
}

require __DIR__.'/base-stemmer.php';
require $phpfile;

$class = 'Snowball'.implode( '', array_map( 'ucfirst', explode('_',$lang) ) ).'Stemmer';
if( ! class_exists($class) ){
    fwrite(STDERR, "$class not included from $phpfile\n");
}
$stemmer = new $class;


fwrite(STDERR,"Waiting for stdin...\n");
$in = fopen('php://stdin', 'r');
fwrite(STDERR,"Stemming $lang...\n");
while( $word = fgets($in) ) {
    $word = rtrim($word, "\n");
    $stem = $stemmer->stemWord($word);
    //fwrite(STDERR,"$word => $stem\n");
    echo $stem,"\n";
}
fwrite(STDERR,"Done\n");
fclose($in);
