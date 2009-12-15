#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT(irstlm, 5.30.00, bertoldi@fbk.eu or federico@fbk.eu)
AM_INIT_AUTOMAKE(irstlm, 5.30.00)

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB


# Checks for libraries.
AC_CHECK_LIB([m], [cos])
AC_CHECK_LIB([z], [gzopen])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ftruncate memmove memset munmap pow strchr strcspn strdup strrchr strspn])

MY_ARCH=`uname -m`
AC_SUBST(libdir,$prefix/lib/$MY_ARCH)
AC_SUBST(bindir,$prefix/bin/$MY_ARCH)
AC_SUBST(wrapperbindir,$prefix/bin)
AC_SUBST(basebindir,$prefix/bin)
AC_SUBST(baselibdir,$prefix/lib)
AC_SUBST(bininstalldir,irstlm-$PACKAGE_VERSION)
AC_SUBST(libinstalldir,irstlm-$PACKAGE_VERSION)

AC_SUBST(MY_MACHTYPE, $MACHTYPE)
AC_SUBST(MY_ARCH, $MY_ARCH)
AC_SUBST(program_transform_name, 's/_lm/-lm/;')

AM_CONDITIONAL(SAME_MACHTYPE, test $MY_MACHTYPE == $MY_ARCH)

AC_CONFIG_FILES([Makefile src/Makefile scripts/Makefile example/Makefile])
AC_OUTPUT
