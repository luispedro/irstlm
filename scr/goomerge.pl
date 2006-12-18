#! /usr/bin/perl

#merge prefix google LMs into one single file and estimate
#processes files level by level

#usage: cat sorted-google-lms | googmerge.pl <path/file>

#create one file per n-gram size, and finally join them 
#all into one single file


$path=($ARGV[0]?$ARGV[0]:"");           #path of existing sub-LM files
$LMFile=($ARGV[1]?$ARGV[1]:"");   #output file to be created
$MAXLEV=($ARGV[2]?$ARGV[2]:0);               #max level of LM

die "usage: goomerge.pl sub-lm-path output-lm ngram-level \n" 
  if !($MAXLEV && $LMFile && $path); 

warn "running with sub-lm-path=$path output-lm=$LMFile ngram-level=$MAXLEV\n";

$gzip="/usr/bin/gzip";   
$gunzip="/usr/bin/gunzip";


warn "compute n-gram sizes\n";
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


warn "build final LM\n";

$LMFile.=".gz" if $LMFile!~/.gz$/;
open(LM,"|$gzip -c > $LMFile") || die "Cannot open $LMFile\n";

#print header
printf LM "ARPA\n\n\\data\\\n";
for ($n=1;$n<=$MAXLEV;$n++){
  printf LM "ngram $n= $size[$n]\n";
}
 
for ($n=1;$n<=$MAXLEV;$n++){
  
  warn "level $n\n";
  
  @files=map { glob($_) } "${path}*.${n}gr*";
  $files=join(" ",@files);
  open(INP,"$gunzip -c $files|") || die "cannot open $f\n";

  printf LM "\\$n-grams:\n";
  while(<INP>){
    @e=split(" ",$_);chop;
    if ($n==1){     
      $e[$#e]=(log($e[$#e])-log($tot1gr))/log(10.0);
    };

    if ($n<$MAXLEV){
      printf LM "%f %s %s\n",$e[$#e],join(" ",@e[1..$#e-1]),$e[0];	
    }
    else{
      printf LM "%f %s\n",$e[$#e],join(" ",@e[0..$#e-1]);	
    }
  }
  
  close(INP);

}
printf LM "\\end\\\n";
close(LM);


