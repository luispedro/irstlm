#!/bin/bash -f
#
#    File:   machine-type
#    Author: The SRI DECIPHER (TM) System
#    Date:   Thu Apr 13 17:01:24 1995
#
#    Description:
#      Determine machine type (sparc, mips, etc.) by
#      running a program common to all UNIX systems,
#      which will query the machine and return its
#      identity. (Based on the Corona script of the same name)
#
#
#    Copyright (c) 1995, SRI International.  All Rights Reserved.
#
#    RCS ID: $Id: machine-type.bash,v 1.10 2006/03/04 06:25:37 stolcke Exp $
#
#    $Log: machine-type.bash,v $
#    Revision 1.11  2009/04/13 bertoldi
#    bash version of the csh version of machine-type script from SRILM
#    NOTE: Under MAC OSX: incompatible with bash versions older than 3.0  
#
#    Revision 1.10  2006/03/04 06:25:37  stolcke
#    return i686 on x86_64 machines
#
#    Revision 1.9  2005/09/30 22:55:35  stolcke
#    support x86_64 platform
#
#    Revision 1.8  2005/09/20 04:47:36  stolcke
#    support ppc64
#
#    Revision 1.7  2003/04/19 16:57:36  stolcke
#    freebsd support
#
#    Revision 1.6  2003/02/21 22:04:38  stolcke
#    support for MacOSX
#
#    Revision 1.5  2002/07/09 15:06:17  stolcke
#    handle i386-solaris case
#
#    Revision 1.4  2002/02/11 08:26:43  stolcke
#    detect cygwin platform
#
#    Revision 1.3  1999/10/22 08:29:18  stolcke
#    fixed syntax errors
#    removed reference to DECIPHER
#    recognizer IRIX6 systems
#
#    Revision 1.2  1999/02/20 07:15:40  stolcke
#    added support for linux/i686
#
# Revision 1.2  1995/04/22  03:25:34  stolcke
# use 'uname -m' (hardware name) to determine cpu type
# (since the same OS may in fact support several cpu types)
#
# Revision 1.1  1995/04/22  01:06:00  tmk
# Initial revision
#
#

##    NOTE:  (tmk 950414)
##	  Usually this script is called in a line like 
##		> setenv MACHINE_TYPE `$DECIPHER/bin/machine-type`
##	  So you want to generate a visible warning if this script can't
##	  figure out the right string to return.  Returning a string
##	  like "ERROR: Unsupported machine type: "$RESULT will silently
##	  push the problem further down the line, as now the environment
##	  variable MACHINE_TYPE is defined as "ERROR:....."  The
##	  approach I've taken generates the message 
##	  "MACHINE_TYPE: Undefined variable."
##	  and leaves the MACHINE_TYPE variable defined as a null string.


UNAME_A=$(uname -a)
UNAME_M=$(uname -m)
UNAME_S=$(uname -s)

set -- $UNAME_A

i=0;
while [[ $# -gt 0 ]] ; do
RESULT[$i]=$1; shift
i=$(( $i + 1 ))
done

if [[ ${#RESULT[@]} -gt 0 ]] ; then
    if [[ ${RESULT[5]} =~ IP* ]] ; then	# "IP" is an irix processor
	if [[ $RESULT[3] =~ 4.* ]] ; then
	    MACHINE_TYPE=mips
	elif [[ $RESULT[3] =~ [56].* ]] ; then
            MACHINE_TYPE=mips-elf
	else
	    ## Generate an error by doing nothing.  (Used to be the line below:)
	    echo "ERROR: Unsupported machine type: "$RESULT  
	fi

    elif [[ $RESULT[5] == sun4* ]] ; then

#	MACHINE_TYPE=sparc

	if  [[ $RESULT[3] =~ 4.* ]] ; then
	    MACHINE_TYPE=sparc
	elif [[ $RESULT[3] =~ 5.* ]] ; then
	    MACHINE_TYPE=sparc-elf
	else
	    ## Generate an error by doing nothing.  (Used to be the line below:)
	    echo "ERROR: Unsupported machine type: "$RESULT
	fi
    elif [[ $RESULT[5] == i86pc ]] ; then
	MACHINE_TYPE=i386-solaris
    elif [[ $RESULT[5] == alpha ]] ; then
	MACHINE_TYPE=alpha
    elif [[ ${UNAME_M} == ppc64 ]] ; then
	MACHINE_TYPE=ppc64
    elif [[ ${UNAME_S} =~ CYGWIN* ]] ; then
	MACHINE_TYPE=cygwin
    elif [[ ${UNAME_S} =~ FreeBSD* ]] ; then
	MACHINE_TYPE=freebsd
    elif [[ ${UNAME_S} =~ Darwin* ]] ; then
	MACHINE_TYPE=macosx
    elif [[ ${UNAME_M} == i686 ]] ; then
	MACHINE_TYPE=i686
    elif [[ ${UNAME_M} == x86_64 ]] ; then
	#MACHINE_TYPE=i686-m64
	MACHINE_TYPE=i686
    else
	## Generate an error by doing nothing.  (Used to be the line below:)
	echo "ERROR: Unsupported machine type: "$RESULT
    fi
else
    ## Generate an error by doing nothing.  (Used to be the line below:)
    echo "ERROR: Unsupported machine type: "$RESULT
fi

## NOTE:  If we couldn't figure out the MACHINE_TYPE by this point, the
##        following line generates an error, rather than returning a
##	  string containing the word "ERROR".  This is still not great 
##	  error handling, but it's slightly better.  

echo $MACHINE_TYPE
