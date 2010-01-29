#!/bin/bash

# NOTE:
# Versions 1.9 (or higher) of aclocal and automake are required.
# Version 2.59 (or higher) of autoconf is required.

# For Mac OSX users:
# Standard distribution usually includes versions 1.6 for aclocal and automake.
# Get versions 1.9 or higher
# Set the following variable to the correct paths
#ACLOCAL="/path/to/aclocal-1.9"
#AUTOMAKE="/path/to/automake-1.9"

function die () {
  echo "$@" >&2
  exit 1
}

if [ -z "$ACLOCAL" ]
then
    ACLOCAL=`which aclocal`
fi

if [ -z "$AUTOMAKE" ]
then
    AUTOMAKE=`which automake`
fi

if [ -z "$AUTOCONF" ]
then
    AUTOCONF=`which autoconf`
fi

if [ -z "$AUTOHEADER" ]
then
    AUTOHEADER=`which autoheader`
fi

if [ -z "$LIBTOOLIZE" ]
then
    LIBTOOLIZE=`which libtoolize`
fi

echo "Calling $ACLOCAL..."
$ACLOCAL || die "aclocal failed"
echo "Calling $AUTOCONF..."
$AUTOCONF || die "autoconf failed"
echo "Calling $AUTOHEADER..."
$AUTOHEADER || die "autoheader failed"
echo "Calling $LIBTOOLIZE..."
$LIBTOOLIZE || die "libtoolize failed"
echo "Calling $AUTOMAKE..."
$AUTOMAKE --add-missing --gnu || die "automake failed"

echo
echo "You should now be able to configure and build:"
echo "   ./configure [--prefix=/path/to/install]"
echo "   make"
echo "   make install"
echo

