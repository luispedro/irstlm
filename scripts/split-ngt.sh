#! /bin/sh

#usage:
#~/workspace/irstlm/scripts/ngt-split.sh <input> <output> <size> <parts>
#It creates <parts> files (named <output.000>, ... <output.999>)
#containing ngram statistics (of <order> length) in Google format
#These files are a partition of the whole set of ngrams

basedir=~/workspace/irstlm/
bindir=$basedir/bin/$MACHTYPE
scriptdir=$basedir/scripts

inputfile=$1; shift
outputfile=$1; shift
order=$1; shift
parts=$1; shift

dictfile=dict$$

$bindir/dict -i=$inputfile -o=$dictfile -f=y -sort=n
$scriptdir/split-dict.pl $dictfile ${dictfile}. $parts

rm $dictfile

for d in `ls ${dictfile}.*` ; do
w=`echo $d | perl -pe 's/.+(\.[0-9]+)$/$1/i'`
w="$outputfile$w"
$bindir/ngt -i=$inputfile  -n=$order -google=y -o=$w -fd=$d  > /dev/null
rm $d
done

exit
