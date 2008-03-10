#!/bin/sh

#usage:
#~/workspace/irstlm/scripts/ngt-split.sh <input> <output> <size> <parts>
#It creates <parts> files (named <output.000>, ... <output.999>)
#containing ngram statistics (of <order> length) in Google format
#These files are a partition of the whole set of ngrams

basedir=/voxgate/ssi/cettolo/irstlm/
bindir=$basedir/bin/$MACHTYPE
scriptdir=$basedir/scripts


unset par
while [ $# -gt 0 ]
do
   echo "$0: arg $1"
   par[${#par[@]}]="$1"
   shift
done

inputfile=${par[0]}
outputfile=${par[1]}
order=${par[2]}
parts=${par[3]}


queue_parameters="-q 64bit-8gb.q -l mem_free=2G"

pwdcmd="pwd"
cmd=`which pawd | head -1 | awk '{print $1}'`
if [ $cmd -a -e $cmd ] ; then pwdcmd=$cmd ; fi
workingdir=`$pwdcmd`
cd `pwd`

qsubout=`pwd`"/OUT$$"
qsuberr=`pwd`"/ERR$$"
qsublog=`pwd`"/LOG$$"
qsubname="NGT"

dictfile=dict$$

qsub $queue_parameters -b no -j yes -sync yes -o /dev/null -e /dev/null -N dict << EOF
cd $workingdir
$bindir/dict -i="$inputfile" -o=$dictfile -f=y -sort=n >& log_dict
$scriptdir/split-dict.pl --input $dictfile --output ${dictfile}. --parts $parts >& log_split-dict
EOF


unset suffix
unset getpids
#getting list of suffixes
for file in `ls ${dictfile}.*` ; do
sfx=`echo $file | perl -pe 's/^.+\.(\d+)$/$1/'`
suffix[${#suffix[@]}]=$sfx
done

#submitting jobs
for sfx in ${suffix[@]} ; do

(\
qsub $queue_parameters -b no -j yes -sync no -o $qsubout.split-$sfx -e $qsuberr.split-$sfx -N $qsubname-$sfx << EOF
cd $workingdir
echo exit status $?
$bindir/ngt -i="$inputfile" -n=$order -gooout=y -o=${outputfile}.$sfx -fd=${dictfile}.$sfx  >& log_ngt-$sfx
echo exit status $?
echo
EOF
) >& $qsublog.split-$sfx

id=`cat $qsublog.split-$sfx | grep 'Your job' | awk '{print $3}'`
sgepid[${#sgepid[@]}]=$id

done

waiting=""
for id in ${sgepid[@]} ; do waiting="$waiting -hold_jid $id" ; done

qsub $queue_parameters -sync yes $waiting -j y -o /dev/null -e /dev/null -N $qsubname.W -b y /bin/ls >& $qsubname.W.log


#checking whether all commands in the submitted jobs worked fine
failstatus=0;
for sfx in ${suffix[@]} ; do
for failure in `cat $qsubout.split-$sfx | grep 'exit status' | awk '{print $3}'` ; do
if [ $failure != 0 ] ; then
echo "submitted job ($qsubout.split-$sfx) died not correctly"
failstatus=1
fi
done
done

#if failure of any job, kill all submitted jobs
if [ $failstatus == 1 ] ; then
for id in ${getpid[@]} ; do qdel $id ; done
fi

#merging output data
for sfx in ${suffix[@]} ; do
cat OUTPUT$$.split-$sfx >> OUTPUT$$
if [ -e $qsubout.split-$sfx ] ; then cat $qsubout.split-$sfx >> $qsubout ; fi 
if [ -e $qsuberr.split-$sfx ] ; then cat $qsuberr.split-$sfx >> $qsuberr ; fi 
if [ -e $qsublog.split-$sfx ] ; then cat $qsublog.split-$sfx >> $qsublog ; fi 
done

#removing all temporary files
for sfx in ${suffix[@]} ; do
rm INPUT$$.split-$sfx
rm OUTPUT$$.split-$sfx
if [ -e $qsubout.split-$sfx ] ; then rm $qsubout.split-$sfx ; fi
if [ -e $qsuberr.split-$sfx ] ; then rm $qsuberr.split-$sfx ; fi
if [ -e $qsublog.split-$sfx ] ; then rm $qsublog.split-$sfx ; fi
done
rm $qsubname.W.log
