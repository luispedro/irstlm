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
my ($w,$f,$totf,$thr);
my (@D,@F,%S,@C);
open(IN,"$input");
while(chomp($_=<IN>)){
	if (/dictionary/i){
		my ($dummy,$size);
		($dummy,$dummy,$size)=split(/[ \t]+/,$_);
		$freqflag=1 if /DICTIONARY/;
		next;
	}
	if ($freqflag){
		($w,$f)=split(/[ \t]+/,$_);
	}
	else{
		$w=$_;
		$f=1;
	}
	push @D, $w}=$f;
	push @F, $w;
$totf+=$f;
#	print STDERR "$w , $D{$w} , $totf\n";
}
close (IN);

$thr=$totf/$parts;
print STDERR "thr: $thr , $totf , $parts\n";

my $sfx=0;
my $w;
$totf=0;
for (my $i=0;$i<=$#D;$i++){
  $totf+=$F[$i];
	$w=$D[$i];
	$S{$w}=$sfx;
	$C[$sfx]++;
#	print STDERR "$w , $i, $F[$i] , $totf , $S{$w} , $C[$sfx]\n";
	if ($totf>$thr){
		$totf=0;
		$sfx++;
	}
}

my $oldsfx=-1;
for (my $i=0;$i<=$#D;$i++){
	$w=$D[$i];
	$sfx="0000$S{$w}";
	$sfx=~s/.+(\d{3})/$1/;
#	print STDERR "$w , $sfx , $S{$w}\n";
	if ($sfx != $oldsfx){
print STDERR "opening $output$sfx\n";
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


