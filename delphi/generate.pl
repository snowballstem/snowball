#!/usr/bin/perl -w
use strict;

my @sources = qw(Danish Dutch English Finnish French German German2 Italian
                 Lovins Norwegian Porter Portuguese Russian Spanish Swedish Dos2Latin);

my $snowball = "../snowball";

foreach my $source(@sources)
{
    # Compile stemmer.
    my $name   = $source."Stemmer";
    my $ifile  = "../algorithms/".$source."/stem.sbl";
    my $prefix = $source."_";

    # Generate Delphi source.
    system("$snowball $ifile -d -o $name");

    # Generate Delphi project file.
    my $src_file = "Test.dpr";
    my $dst_file = sprintf('%s%s', $source, $src_file);

    open(SRC,"< $src_file") or die($!);
    open(DST,"> $dst_file") or die($!);
    
    while(defined(my $line=<SRC>)){
        chomp($line);
        $line =~ s/%STEMMER%/$source/g;
        print DST "$line\n";
    }

    close(DST) or warn($!);
    close(SRC) or warn($!);
    
    # Compile Delphi project.
    system("dcc32 -Q $dst_file");


    # Generate C source.
#    system("$snowball $ifile -o $name -ep $prefix -r ../runtime");

    # Generate C driver file.
#    my $src_file = "driver.c";
#    my $dst_file = sprintf("%sTest.c", $source);

#    open(SRC,"< $src_file") or die($!);
#    open(DST,"> $dst_file") or die($!);

#    while(defined(my $line=<SRC>)){
#        chomp($line);
#        $line =~ s/%STEMMER%/$source/g;
#        print DST "$line\n";
#    }

#    close(DST) or warn($!);
#    close(SRC) or warn($!);

    # Compile C source.
#    my $c_dst = $source."Test_C.exe";
#    system("gcc -o $c_dst $dst_file $name.c ../libstemmer.o");
}

