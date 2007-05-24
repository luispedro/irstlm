#! /bin/sh

usage()
{
cat << EOF
usage: $0 options

This script estimates a language model file. 

OPTIONS:
   -h      Show this message
   -i      Input training file e.g. 'gunzip -c train.gz'
   -o      Output gzipped LM, e.g. lm.gz
   -k      Number of splits (default 5)
   -n      Order of language model (default 3)
   -t      Directory for temporary files (default ./stat)
   -p      Prune singleton n-grams (default false)
   -s      Smoothing methods: witten-bell (default), kneser-ney (approximated kneser-ney)
   -v      Verbose

EOF
}

if [ ! $IRSTLM ]; then
   echo "Set IRSTLM environment variable with path to irstlm"
   exit;
fi

#paths to scripts and commands in irstlm
scr=$IRSTLM/bin/
bin=$IRSTLM/bin/$MACHTYPE

#check irstlm installation
if [ ! -e $bin/dict -o  ! -e $scr/split-dict.pl ]; then
   echo "$IRSTLM does not contain a proper installation of IRSTLM"
   exit;
fi

#default parameters
logfile=/dev/null
tmpdir=stat
order=3
parts=3
inpfile="";
outfile=""
verbose="";
smoothing="--witten-bell";
prune="";
boundaries="";

while getopts “hvi:o:n:k:t:s:pbl:” OPTION
do
     case $OPTION in
         h)
             usage
             exit 1
             ;;
         v)
             verbose=1;
             ;;
         i)
             inpfile=$OPTARG
             ;;
         o)
             outfile=$OPTARG
             ;;
         n)
             order=$OPTARG
             ;;
         k)
             parts=$OPTARG
             ;;
         t)
             tmpdir=$OPTARG
             ;;
         s)
             smoothing=$OPTARG
	     case $smoothing in
	     witten-bell) 
		     smoothing="--witten-bell"
		     ;; 
	     kneser-ney)
		     smoothing="--kneser-ney"
		     ;;
	     *) 
		 echo "wrong smoothing setting";
		 exit;
	     esac
             ;;
         p)
             prune='--prune-singletons';
             ;;
         b)
             boundaries='--cross-sentence';
             ;;
	 l)
             logfile=$OPTARG
             ;;
         ?)
             usage
             exit
             ;;
     esac
done


if [ $verbose ];then
echo inpfile=\"$inpfile\" outfile=$outfile order=$order parts=$parts tmpdir=$tmpdir prune=$prune smooth=$smoothing
fi

if [ ! "$inpfile" -o ! "$outfile" ]; then
    usage
    exit 
fi
 
if [ -e $outfile ]; then
   echo "Output file $outfile already exists! either remove or rename it."
   exit;
fi

if [ -e $logfile -a $logfile!="/dev/null" ]; then
   echo "Logfile $logfile already exists! either remove or rename it."
   exit;
fi

#check tmpdir
if [ ! -d $tmpdir ]; then
   echo "Temporary directory $tmpdir not found";
   exit;
else
    echo "Cleaning temporary directory $tmpdir";
    rm $tmpdir/dict* $tmpdir/ngram.dict.* $tmpdir/lm.dict.* >& /dev/null

fi




echo "Extracting dictionary from training corpus"
$bin/dict -i="$inpfile" -o=$tmpdir/dictionary -f=y >& $logfile

echo "Splitting dictionary into $parts lists"
$scr/split-dict.pl --input $tmpdir/dictionary --output $tmpdir/dict. --parts $parts >> $logfile 2>&1

echo "Extracting n-gram statistics for each word list"
for sdict in $tmpdir/dict.*;do
sdict=`basename $sdict $tmpdir`
echo $sdict;
$bin/ngt -i="$inpfile" -n=$order -gooout=y -o="gzip -c > $tmpdir/ngram.${sdict}.gz" -fd="$tmpdir/$sdict"  >> $logfile 2>&1
done

echo "Estimating language models for each word list"
for sdict in $tmpdir/dict.*;do
sdict=`basename $sdict $tmpdir`
echo $sdict;
$scr/build-sublm.pl $prune $smooth --size $order --ngrams "gunzip -c $tmpdir/ngram.${sdict}.gz" -sublm $tmpdir/lm.$sdict  >> $logfile 2>&1
done

echo "Merging language models into $outfile"
$scr/merge-sublm.pl --size $order --sublm $tmpdir/lm.$sdict -lm $outfile  >> $logfile 2>&1

echo "Cleaning temporary directory $tmpdir";
#rm $tmpdir/dict* $tmpdir/ngram.dict.* $tmpdir/lm.dict.* >& /dev/null

exit


