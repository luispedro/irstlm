#! /usr/bin/perl

#first pass: read full input and generate 1-grams
#second pass: 
#for n=2 to N
#  foreach n-1-grams
#      foreach  n-grams with history n-1
#          compute smoothing statistics
#          store successors
#      compute back-off probability
#      compute smoothing probability
#      write n-1 gram with back-off prob 
#      write all n-grams with smoothed probability


$N=5; #maximum order of n-grams
$cntbias=39; #remove this value from n-gram counts
$path=$ARGV[0]; #suffix of the LM tables to be generated;
$input=$ARGV[1]; #file or command with the n-gram table

$gzip="/usr/bin/gzip";
$gunzip="/usr/bin/gunzip";


$log10=log(10.0);

warn "collecting 1-gram counts\n";

open(INP,"$input") || open(INP,"$input|")  || die "cannot open $input\n";

$oldwrd=""; $code=-1; @cnt=();

while ($ng=<INP>){

chop $ng; @ng=split(/[ \t]/,$ng); $ngcnt=(pop @ng) - $cntbias;

if ($oldwrd ne $ng[0]){
    $oldwrd=$ng[0];$dict[++$code]=$ng[0];
}

#update counter
$cnt[$code]+=$ngcnt;
$totcnt+=$ngcnt;
}

close(INP);

open(GR,"|$gzip -c >${path}.1gr.gz") || die "cannot create ${path}.1gr.gz\n";
for ($c=0;$c<=$code;$c++){
    #printf GR "%s %f\n",$dict[$c],log($cnt[$c]/$totcnt)/$log10;
    printf GR "%s %s\n",$dict[$c],$cnt[$c];
}
close(GR);


foreach ($n=2;$n<=$N;$n++){
  
  warn "computing $n-gram probabilities\n"; 
  open(HGR,"$gunzip -c ${path}.".($n-1)."gr.gz|") || die "cannot open ${path}.".($n-1)."gr.gz\n";
  open(INP,"$input") || open(INP,"$input|")  || die "cannot open $input\n";
  open(GR,"|$gzip -c >${path}.${n}gr.gz");
  open(NHGR,"|$gzip -c > ${path}.".($n-1)."ngr.gz") || die "cannot open ${path}.".($n-1)."ngr.gz";

  chop($ng=<INP>); @ng=split(/[ \t]/,$ng);$ngcnt=(pop @ng) - $cntbias;
  chop($h=<HGR>);  @h=split(/ /,$h); $hpr=pop @h;
  
  $code=-1;@cnt=(); @dict=(); $totcnt=0;$diff=0; $oldwrd="";
   
  do{
    while (join(" ",@h[0..$n-2]) eq join(" ",@ng[0..$n-2])){ #true the first time
        #print join(" ",@h[0..$n-2])," --- ", join(" ",@ng[0..$n-2])," $ngcnt \n";    

        #collect Witten Bell smoothing statistics 
        if ($oldwrd ne $ng[$n-1]){$dict[++$code]=$oldwrd=$ng[$n-1];$diff++;}
        $cnt[$code]+=$ngcnt; $totcnt+=$ngcnt;            
 
       #read next ngram
        chop($ng=<INP>); @ng=split(/[ \t]/,$ng);$ngcnt=(pop @ng) - $cntbias;	
      }

     #print smoothed probabilities
     for ($c=0;$c<=$code;$c++){      
       printf GR "%s %s %f\n",join(" ",@h[0..$n-2]),$dict[$c],log($cnt[$c]/($totcnt+$diff))/$log10;
     }

     #rewrite history including back-off weight
     print "$h - $ng - $totcnt $diff \n" if $totcnt+$diff==0;

     printf NHGR "%f %s\n",log($diff/($totcnt+$diff))/$log10,$h;

     #reset smoothing statistics
     $code=-1;@cnt=(); @dict=(); $totcnt=0;$diff=0;$oldwrd="";
  
     #read next history
     chop($h=<HGR>);  @h=split(/ /,$h); $hpr=pop @h;
  
 }until ($ng eq ""); #n-grams are over

 close(HGR); close(INP);close(GR);close(NGR);
 rename("${path}.".($n-1)."ngr.gz","${path}.".($n-1)."gr.gz");
}   


