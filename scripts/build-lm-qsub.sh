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
   -b      Include sentence boundary n-grams (optional)
   -d      Define subdictionary for n-grams (optional)
   -v      Verbose

EOF
}

if [ ! $IRSTLM ]; then
   echo "Set IRSTLM environment variable with path to irstlm"
   exit;
fi

#paths to scripts and commands in irstlm
scr=$IRSTLM/bin/
bin=$IRSTLM/bin

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
dictionary="";

while getopts “hvi:o:n:k:t:s:pbl:d:” OPTION
do
     case $OPTION in
         h)
             usage
             exit 1
             ;;
         v)
             verbose="--verbose";
             ;;
         i)
             inpfile=$OPTARG
             ;;
         d)
             dictionary="-sd=$OPTARG"
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
echo inpfile=\"$inpfile\" outfile=$outfile order=$order parts=$parts tmpdir=$tmpdir prune=$prune smoothing=$smoothing dictionary=$dictionary
fi

if [ ! "$inpfile" -o ! "$outfile" ]; then
    usage
    exit 
fi
 
if [ -e $outfile ]; then
   echo "Output file $outfile already exists! either remove or rename it."
   exit;
fi

if [ -e $logfile -a $logfile != "/dev/null" ]; then
   echo "Logfile $logfile already exists! either remove or rename it."
   exit;
fi

#check tmpdir
if [ ! -d $tmpdir ]; then
   echo "Temporary directory $tmpdir not found";
   echo "creating $tmpdir";
   mkdir -p $tmpdir;
else
    echo "Cleaning temporary directory $tmpdir";
    rm $tmpdir/dict* $tmpdir/ngram.dict.* $tmpdir/lm.dict.* >& /dev/null
fi


queue_parameters="-q 64bit-8gb.q -l mem_free=2G"

pwdcmd="pwd"
cmd=`which pawd | head -1 | awk '{print $1}'`
if [ $cmd -a -e $cmd ] ; then pwdcmd=$cmd ; fi
workingdir=`$pwdcmd`
cd `pwd`

qsub $queue_parameters -b no -j yes -sync yes -o /dev/null -e /dev/null -N dict << EOF
export MACHTYPE=$MACHTYPE
echo exit status $?
cd $workingdir
echo exit status $?
echo "Extracting dictionary from training corpus"
$bin/dict -i="$inpfile" -o=$tmpdir/dictionary -f=y -sort=no >& log_dict
echo exit status $?
echo "Splitting dictionary into $parts lists"
$scr/split-dict.pl --input $tmpdir/dictionary --output $tmpdir/dict. --parts $parts >& log_split-dict
echo exit status $?
EOF

unset suffix
#getting list of suffixes
for file in `ls $tmpdir/dict.*` ; do
sfx=`echo $file | perl -pe 's/^.+\.(\d+)$/$1/'`
suffix[${#suffix[@]}]=$sfx
done

qsubout=`pwd`"/OUT$$"
qsuberr=`pwd`"/ERR$$"
qsublog=`pwd`"/LOG$$"
qsubname="NGT"

unset getpids
echo "Extracting n-gram statistics for each word list"
for sfx in ${suffix[@]} ; do

(\
qsub $queue_parameters -b no -j yes -sync no -o $qsubout.$sfx -e $qsuberr.$sfx -N $qsubname-$sfx << EOF
export MACHTYPE=$MACHTYPE
echo exit status $?
cd $workingdir
echo exit status $?
$bin/ngt -i="$inpfile" -n=$order -gooout=y -o="gzip -c > $tmpdir/ngram.dict.${sfx}.gz" -fd="$tmpdir/dict.${sfx}" >& log_ngt-$sfx
echo exit status $?
echo
EOF
) >& $qsublog.$sfx

id=`cat $qsublog.$sfx | grep 'Your job' | awk '{print $3}'`
sgepid[${#sgepid[@]}]=$id

done

waiting=""
for id in ${sgepid[@]} ; do waiting="$waiting -hold_jid $id" ; done

qsub $queue_parameters -sync yes $waiting -j y -o /dev/null -e /dev/null -N $qsubname.W -b y /bin/ls >& $qsubname.W.log


qsubout=`pwd`"/OUT$$"
qsuberr=`pwd`"/ERR$$"
qsublog=`pwd`"/LOG$$"
qsubname="SUBLM"

unset getpids
echo "Estimating language models for each word list"
for sfx in ${suffix[@]} ; do
(\
qsub $queue_parameters -b no -j yes -sync no -o $qsubout.$sfx -e $qsuberr.$sfx -N $qsubname-$sfx << EOF
export MACHTYPE=$MACHTYPE
echo exit status $?
cd $workingdir
echo exit status $?
$scr/build-sublm.pl $verbose $prune $smoothing --size $order --ngrams "gunzip -c $tmpdir/ngram.dict.${sfx}.gz" -sublm $tmpdir/lm.dict.${sfx}  >> $logfile 2>&1
echo exit status $?
echo
EOF
) >& $qsublog.$sfx

id=`cat $qsublog.$sfx | grep 'Your job' | awk '{print $3}'`
sgepid[${#sgepid[@]}]=$id

done

waiting=""
for id in ${sgepid[@]} ; do waiting="$waiting -hold_jid $id" ; done

qsub $queue_parameters -sync yes $waiting -j y -o /dev/null -e /dev/null -N $qsubname.W -b y /bin/ls >& $qsubname.W.log


echo "Merging language models into $outfile"
qsub $queue_parameters -b no -j yes -sync yes -o /dev/null -e /dev/null -N merge << EOF
export MACHTYPE=$MACHTYPE
echo exit status $?
cd $workingdir
$scr/merge-sublm.pl --size $order --sublm $tmpdir/lm.dict -lm $outfile  >& log_merge
EOF

echo "Cleaning temporary directory $tmpdir";
#rm $tmpdir/dict* $tmpdir/ngram.dict.* $tmpdir/lm.dict.* >& /dev/null
rm $qsubout* $qsuberr* $qsublog*

exit


