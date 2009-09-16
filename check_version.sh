#!/bin/sh
echo "checking autotools..."

# autoconf 2.59 or newer
ac_version=`${AUTOCONF:-autoconf} --version 2>/dev/null|sed -e 's/^[^0-9]*//;s/[a-z]* *$//;s/[- ].*//g;q'`
if test -z "$ac_version"; then
echo "bootstrap: autoconf not found."
echo "           You need autoconf version 2.59 or newer installed"
echo "           to build IRSTLM from SVN."
echo "           Please install autoconf version 2.59 or newer and set AUTOCONF variable accordingly."
exit 1
fi
IFS=_; set $ac_version; IFS=' '
ac_version=$1
IFS=.; set $ac_version; IFS=' '
if test "$1" = "2" -a "$2" -lt "59" || test "$1" -lt "2"; then
echo "bootstrap: autoconf version $ac_version found."
echo "           You need autoconf version 2.59 or newer installed"
echo "           to build IRSTLM from SVN."
echo "           Please install autoconf version 2.59 or newer and set AUTOCONF variable accordingly."
exit 1
else
echo "bootstrap: autoconf version $ac_version (ok)"
fi

# automake 1.9 or newer
am_version=`${AUTOMAKE:-automake} --version 2>/dev/null|sed -e 's/^[^0-9]*//;s/[a-z]* *$//;s/[- ].*//g;q'`
if test -z "$am_version"; then
echo "bootstrap: automake not found."
echo "           You need automake version 1.9 or newer installed"
echo "           to build IRSTLM from SVN."
echo "           Please install automake version 1.9 or newer and set AUTOMAKE variable accordingly."
exit 1
fi
IFS=_; set $am_version; IFS=' '
am_version=$1
IFS=.; set $am_version; IFS=' '
if test "$1" = "1" -a "$2" -lt "9"; then
echo "bootstrap: automake version $am_version found."
echo "           You need automake version 1.9 or newer installed"
echo "           to build IRSTLM from SVN."
echo "           Please install automake version 1.9 or newer and set AUTOMAKE variable accordingly."
exit 1
else
echo "bootstrap: automake version $am_version (ok)"
fi

# aclocal 1.9 or newer

acl_version=`${ACLOCAL:-aclocal} --version 2>/dev/null|sed -e 's/^[^0-9]*//;s/[a-z]* *$//;s/[- ].*//g;q'`
if test -z "$acl_version"; then
echo "bootstrap: aclocal not found."
echo "           You need aclocal version 1.9 or newer installed"
echo "           to build IRSTLM from SVN."
echo "           Please install aclocal version 1.9 or newer and set ACLOCAL variable accordingly."
exit 1
fi
IFS=_; set $acl_version; IFS=' '
acl_version=$1
IFS=.; set $acl_version; IFS=' '
if test "$1" = "1" -a "$2" -lt "9"; then
echo "bootstrap: aclocal version $acl_version found."
echo "           You need aclocal version 1.9 or newer installed"
echo "           to build IRSTLM SVN."
echo "           Please install aclocal version 1.9 or newer and set ACLOCAL variable accordingly."
exit 1
else
echo "bootstrap: aclocal version $acl_version (ok)"
fi

