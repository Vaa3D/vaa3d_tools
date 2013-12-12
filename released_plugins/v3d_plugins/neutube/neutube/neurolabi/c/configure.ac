AC_INIT(neurolabi, 1.0, tingzhao@gamil.com)

AC_CONFIG_SRCDIR(tz_utilities.c)
AC_CONFIG_HEADERS([include/neurolabi_config.h:include/neurolabi_config-h.in])

# checks for programs
AC_PROG_CC
AC_PROG_MAKE_SET

# checks for libraries
AC_ARG_ENABLE(fftw, [AC_HELP_STRING([--disable-fftw], [disable fftw])],
    disable_fftw=$enablval, disable_fftw=no)
if test "$disable_fftw" = "no"; then
  AC_CHECK_LIB(fftw3, fftw_execute)
fi

AC_ARG_ENABLE(fftwf, [AC_HELP_STRING([--disable-fftwf], [disable fftwf])],
    disable_fftwf=$enablval, disable_fftwf=no)
if test "$disable_fftwf" = "no"; then
AC_CHECK_LIB(fftw3f, fftwf_execute)
fi

AC_ARG_ENABLE(z, [AC_HELP_STRING([--disable-z], [disable z])],
    disable_z=$enablval, disable_z=no)
if test "$disable_z" = "no"; then
AC_CHECK_LIB(z, compress)
fi

AC_ARG_ENABLE(xml, [AC_HELP_STRING([--disable-xml], [disable xml])],
    disable_xml=$enablval, disable_xml=no)
if test "$disable_xml" = "no"; then
AC_CHECK_LIB(xml2, xmlParseFile)
fi

AC_CHECK_LIB(m, sqrt)

AC_ARG_ENABLE(png, [AC_HELP_STRING([--disable-png], [disable png])],
    disable_png=$enablval, disable_png=no)
if test "$disable_png" = "no"; then
AC_CHECK_LIB(png, png_read_image)
fi

AC_ARG_ENABLE(jansson, [AC_HELP_STRING([--disable-jansson], [disable jansson])],
    disable_jansson=$enablval, disable_jansson=no)
if test "$disable_jansson" = "no"; then
AC_CHECK_LIB(jansson, json_object)
fi

#AC_CHECK_LIB(tiff, TIFFOpen)

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h strings.h sys/time.h unistd.h regex.h dirent.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_C_CONST
AC_C_INLINE
#AC_TYPE_INT16_T
#AC_TYPE_INT32_T
#AC_TYPE_INT64_T
#AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
#AC_TYPE_UINT16_T
#AC_TYPE_UINT32_T
#AC_TYPE_UINT64_T
#AC_TYPE_UINT8_T
#AC_C_VOLATILE

#AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero floor round lrint gettimeofday memmove memset regcomp sqrt strchr strcspn strerror strrchr strspn strtol strsep memset_pattern4 fgetln])

AC_ARG_ENABLE(mylib, [AC_HELP_STRING([--enable-mylib], [use mylib])],
    enable_mylib=$enableval, enable_mylib=no)

AC_ARG_ENABLE(64bit, [AC_HELP_STRING([--enable-64bit], [compile 64-bit])],
    enable_64bit=$enableval, enable_64bit=no)	

AC_ARG_ENABLE(shared, [AC_HELP_STRING([--enable-shared], [compile shared library])],
    enable_shared=$enableval, enable_shared=no)	

if test "$enable_mylib" = "no"; then
    AC_DEFINE(GENELIB_VER, 2, [genelib version])
    GENELIB_DIR='../lib/genelib/src'
    GENELIB_TAG=myers
else
    AC_DEFINE(GENELIB_VER, 3, [genelib version])
    GENELIB_DIR='../lib/mylib'
    GENELIB_TAG=mylib
fi

EXT_LIB_DIR="$HOME/local/lib"
DEBUG_DIR=obj
RELEASE_DIR=release
SHARED_FLAG=""
if test "$enable_shared" = "yes"; then
  SHARED_FLAG="-fPIC"
fi
#echo $SHARED_FLAG

if test "$enable_64bit" = "yes"; then
    AC_DEFINE(ARCH_BIT, 64, [32-bit or 64-bit])
    #EXT_LIB_DIR=/usr/local/64bit/lib
    #ARCH_FLAG='-arch x86_64'
    ARCH_FLAG='-m64'
    #DEBUG_DIR=obj64
    #RELEASE_DIR=release64
    #LIB_FILE=libneurolabi64.a
    #DYLIB_FILE=libneurolabi64.dylib
else
    AC_DEFINE(ARCH_BIT, 32, [32-bit or 64-bit])
    ARCH_FLAG=''
fi

AC_ARG_ENABLE(progress, [AC_HELP_STRING([--enable-progress], [progress bar])],
    enable_prgress=$enableval, enable_progress=no)

if test "$enable_progress" = "no"; then
   AC_DEFINE(INTERFACE_PROGRESS_OFF, 1, [progress bar on or off])
fi

AC_SUBST(EXT_LIB_DIR)
AC_SUBST(ARCH_FLAG)
AC_SUBST(DEBUG_DIR)
AC_SUBST(RELEASE_DIR)
AC_SUBST(GENELIB_DIR)
AC_SUBST(GENELIB_TAG)
AC_SUBST(LIB_FILE)
AC_SUBST(DYLIB_FILE)
AC_SUBST(SHARED_FLAG)

AC_CONFIG_FILES(Makefile-def)
AC_OUTPUT
