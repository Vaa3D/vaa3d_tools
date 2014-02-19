AC_PREREQ(2.61)
AC_INIT(libmyers, 1.0)
AC_CONFIG_SRCDIR([utilities.p])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero ftruncate strdup strtol])

AC_ARG_ENABLE(64bit, [AC_HELP_STRING([--enable-64bit], [compile 64-bit])],
		     enable_64bit=$enableval, enable_64bit=no)
if test "$enable_64bit" = "yes"; then
    AC_DEFINE(ARCH_BIT, 64, [32-bit or 64-bit])
    ARCH_FLAG='-m64'
    #PREFIX='b64'
    #LIBFILE=libmyers64.a
fi

ARCH_FLAG=''
PREFIX=''
LIBFILE=libmyers.a

AC_SUBST(ARCH_FLAG)
AC_SUBST(PREFIX)
AC_SUBST(LIBFILE)

AC_CONFIG_FILES([Makefile_a])
AC_OUTPUT
