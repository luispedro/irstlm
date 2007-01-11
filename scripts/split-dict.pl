#! /usr/bin/perl

#usage:
#split-dict.pl <input> <output> <parts>
#It splits the <input> dictionary into <parts> dictionaries
#(named <output000>, ... <output999>)
#splitting is balanced wrt to frequency of the <input> dictionary
#if not available a frequency of 1 is considered

use strict;

my $input=$ARGV[0]; shift (@ARGV);
my $output=$ARGV[0]; shift (@ARGV);
my $parts=$ARGV[0]; shift (@ARGV);

die "Input $input does not exists!\n" if (! -e $input);

my $freqflag=0;
my ($w,$f,$globf,$thr);
my (@D,@F,%S,@C);
open(IN,"$input");

chomp($_=<IN>);
#if input is a dictionary.
if (/^dictionary[ \t]+\d+[ \t]+\d+$/i){
  my ($dummy,$size);
  ($dummy,$dummy,$size)=split(/[ \t]+/,$_);
  $freqflag=1 if /DICTIONARY/;
}

$globf=0;
while(chomp($_=<IN>)){
	if ($freqflag){
		($w,$f)=split(/[ \t]+/,$_);
	}
	else{
		$w=$_;
		$f=1;
	}
	push @D, $w;
	push @F, $f;
  $globf+=$f;
}
close (IN);

$thr=$globf/$parts;
my $totf=0;
print STDERR "Dictionary 0: (thr: $thr , $globf, $totf , $parts)\n";

my $sfx=0;
my $w;
for (my $i=0;$i<=$#D;$i++){
# if a new word cross over too much the threshold
# do not insert in this subdict (but in the following
	if (($totf+$F[$i])>($thr*(1+1/($parts-$sfx)))){
# recompute threshold on the remaining global frequency
# according to the number of remaining parts
		$sfx++;
		$globf-=$totf;
		$thr=($globf)/($parts-$sfx);
		print STDERR "Dictionary $sfx: (thr: $thr , $globf , $totf , ",($parts-$sfx),")\n";
		$totf=0;
	}
	
  $totf+=$F[$i];
	$w=$D[$i];
	$S{$w}=$sfx;
	$C[$sfx]++;
	if ($totf>$thr){
# recompute threshold on the remaining global frequency
# according to the number of remaining parts
		$sfx++;
		$globf-=$totf;
		$thr=($globf)/($parts-$sfx);
		print STDERR "Dictionary $sfx: (thr: $thr , $globf , $totf , ",($parts-$sfx),")\n";
		$totf=0;
	}
}


my $oldsfx=-1;
for (my $i=0;$i<=$#D;$i++){
	$w=$D[$i];
	$sfx="0000$S{$w}";
	$sfx=~s/.+(\d{3})/$1/;
	if ($sfx != $oldsfx){
#print STDERR "opening $output$sfx\n";
		close (OUT) if $oldsfx!= -1;
		open(OUT,">$output$sfx");
		if ($freqflag){
			print OUT "DICTIONARY 0 $C[$sfx]\n";
		}
		else{
			print OUT "dictionary 0 $C[$sfx]\n";
		}
		$oldsfx=$sfx;
	}
	if ($freqflag){
		print OUT "$w $F[$i]\n";
	}
	else{
		print OUT "$w\n";
	}
}
close (OUT) if $oldsfx!= -1;

my $numdict=$S{$D[$#D]}+1;
die "Only $numdict dictionaries were crested instead of $parts!" if ($numdict != $parts);

