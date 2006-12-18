#! /usr/bin/perl

#merge prefix google LMs into one single file and estimate
#processes files level by level

#usage: cat sorted-google-lms | googmerge.pl <path/file>

#create one file per n-gram size, and finally join them 
#all into one single file


$path="lm-5g";           #path of existing sub-LM files
$LMFile="BigLM-5g.lm";   #output file to be created
$MAXLEV=5;               #max level of LM

$gzip="/usr/bin/gzip";   
$gunzip="/usr/bin/gunzip";


#first go through in order to compute n-gram sizes
@size=();
$tot1gr=0;

for ($n=1;$n<=$MAXLEV;$n++){
  #print "$n\n";

  @files=map { glob($_) } "${path}*.${n}gr*";
  $files=join(" ",@files);
  open(INP,"$gunzip -c $files|") || die "cannot open $f\n";
  while(<INP>){
    $size[$n]++;
    if ($n==1){
      chop;@e=split(" ",$_);
      $tot1gr+=$e[$#e];
    }
  }
  close(INP);
}


#build final LM

$LMFile.=".gz" if $LMFile!~/.gz$/;
open(LM,"|$gzip -c > $LMFile") || die "Cannot open $LMFile\n";

#print header
printf LM "ARPA\n\n\\data\\\n";
for ($n=1;$n<=$MAXLEV;$n++){
  printf LM "ngram $n= $size[$n]\n";
}
 
for ($n=1;$n<=$MAXLEV;$n++){
  
  @files=map { glob($_) } "${path}*.${n}gr*";
  $files=join(" ",@files);
  open(INP,"$gunzip -c $files|") || die "cannot open $f\n";

  printf LM "\\$n-grams:\n";
  while(<INP>){
    if ($n==1){
      chop; 
      @e=split(" ",$_);
      printf LM "%s %f\n",join(" ",@e[0..$#e-1]),
	(log($e[$#e])-log($tot1gr))/log(10.0);

    }
    else{print LM $_};
  }
  close(INP);
}

close(LM);


