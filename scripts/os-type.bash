#!/bin/bash -f
#
#    File:   os-type
#    Author: Nicola Bertoldi
#    Date:   Thu Apr 14 2009
#
#    Description:
#      Determine operating system (Darwin, Linux, etc.) by
#      running a program common to all UNIX systems,
#      which will query the OS and return its
#      identity. (Based on the Corona script of the same name)
#
#
#    Copyright (c) FBK-irst.  All Rights Reserved.
#
#    RCS ID: $Id$
#
#    $Log$
#    Revision 1.01  2009/04/14  bertoldi
#    Initial revision
#    Support for Darwin and Linux
#
#

##    NOTE:  (tmk 950414)
##	  Usually this script is called in a line like 
##		> setenv OS_TYPE `$DECIPHER/bin/os-type`
##	  So you want to generate a visible warning if this script can't
##	  figure out the right string to return.  Returning a string
##	  like "ERROR: Unsupported machine type: "$RESULT will silently
##	  push the problem further down the line, as now the environment
##	  variable OS_TYPE is defined as "ERROR:....."  The
##	  approach I've taken generates the message 
##	  "OS_TYPE: Undefined variable."
##	  and leaves the MACHINE_TYPE variable defined as a null string.


UNAME_S=$(uname -s)

if [[ ${UNAME_S} =~ Linux* ]] ; then
	OS_TYPE=Linux
elif [[ ${UNAME_S} =~ Darwin* ]] ; then
	OS_TYPE=Darwin

else
## Generate an error by doing nothing.  (Used to be the line below:)
echo "ERROR: Unsupported machine type: "$RESULT
fi


## NOTE:  If we couldn't figure out the OS_TYPE by this point, the
##        following line generates an error, rather than returning a
##	  string containing the word "ERROR".  This is still not great 
##	  error handling, but it's slightly better.  

echo $OS_TYPE
