#! /usr/bin/perl

#merge prefix LMs into one single file

use strict;
use Getopt::Long "GetOptions";

my ($help,$lm,$size,$sublm)=();
$help=1 unless
&GetOptions('size=i' => \$size,
            'lm=s' => \$lm,
            'sublm=s' => \$sublm,
            'help' => \$help,);


if ($help || !$size || !$lm || !$sublm){
  print "merge-sublm.pl <options>\n",
  "--size <int>        maximum n-gram size for the language model\n",
  "--sublm <string>    path identifying  all prefix sub LMs \n",
  "--lm <string>       name of final LM file (will be gzipped)\n",
  "--help              (optional) print these instructions\n";    
  exit(1);
}


my $gzip="/usr/bin/gzip";   
my $gunzip="/usr/bin/gunzip";


warn "Compute total sizes of n-grams\n";
my @size=();         #number of n-grams for each level
my $tot1gr=0;        #total frequency of 1-grams
my $pr;              #probability of 1-grams
my (@files,$files);  #sublm files for a given n-gram size  

for (my $n=1;$n<=$size;$n++){

  @files=map { glob($_) } "${sublm}*.${n}gr*";
  $files=join(" ",@files);
  open(INP,"$gunzip -c $files|") || die "cannot open $files\n";
  while(<INP>){
    $size[$n]++;
    if ($n==1){
      chop;split(" ",$_);
      $tot1gr+=$_[0];
    }
  }
  close(INP);
}


warn "Merge all sub LMs\n";

$lm.=".gz" if $lm!~/.gz$/;
open(LM,"|$gzip -c > $lm") || die "Cannot open $lm\n";

warn "Write LM Header\n";

printf LM "ARPA\n\n\\data\\\n";
for (my $n=1;$n<=$size;$n++){
  printf LM "ngram $n= $size[$n]\n";
}

warn "Writing LM Tables\n";
for (my $n=1;$n<=$size;$n++){
  
  warn "Level $n\n";
  
  @files=map { glob($_) } "${sublm}*.${n}gr*";
  $files=join(" ",@files);
  open(INP,"$gunzip -c $files|") || die "cannot open $files\n";

  printf LM "\\$n-grams:\n";
  while(<INP>){   
    
    if ($n==1){         
      split(" ",$_);
      $pr=(log($_[0])-log($tot1gr))/log(10.0);shift @_;
      printf LM "%f %s\n",$pr,join(" ",@_);
    }
    else{
      printf LM "%s",$_;
    }
  }
  
  close(INP);
}

printf LM "\\end\\\n";
close(LM);


