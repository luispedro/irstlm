#! /usr/bin/perl

#re-segment n-gram count files into files so that
#n-grams starting with a given word (prefix) are all 
#contained in one file.


$max_pref=2;#10000;   #number of prefixes to be put in one file 
$max_ngram=100;#5000000;#number of n-grams to be put in one file
$file_cnt=0;       #counter of files 
$pref_cnt=0;       #counter of prefixes in the current file
$ngram_cnt=0;      #counter of n-gram in the current file   

$path=($ARGV[0]?$ARGV[0]:"goong");     #path of files to be created
$gzip="/bin/gzip";

$pwrd="";
open(OUT,sprintf("|$gzip -c > %s.%04d.gz",$path,++$file_cnt));
while ($ng=<STDIN>){
  ($wrd)=$ng=~/^([^ ]+)/;
  #warn "$wrd\n";
  if ($pwrd ne $wrd){
    $pwrd=$wrd;
    if ($file_pref>$max_pref || $ngram_cnt>$max_ngram){
      warn "it's time to change file\n";
      close(OUT);
      open(OUT,sprintf("|$gzip -c > %s.%04d.gz",$path,++$file_cnt));
      $pref_cnt=$ngram_cnt=0;
    }
    else{
      $pref_cnt++;
    }
  }
  print OUT $ng;
  $ngram_cnt++;
}
close(OUT);

