AC_PREREQ([2.67])
AC_INIT([minidjvu-mod],[0.9m01],[trufanovan@gmail.com],,[https://github.com/trufanov-nok/minidjvu_mod/])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h:config/config.h.in)
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT(/usr/local)

AM_INIT_AUTOMAKE([subdir-objects foreign -Wall])

AC_LANG([C++])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PATH_PROG(GZIP, gzip)
AC_PATH_PROG(RM, rm)
AM_PROG_AR

LT_INIT

PKG_INSTALLDIR

# Checks for libraries.

AC_CHECK_LIB(z, inflate)
AC_CHECK_LIB(jpeg, jpeg_destroy_decompress)
AC_CHECK_LIB(tiff, TIFFOpen)
AC_CHECK_LIB(jemalloc,malloc)
# Check for OpenMP
AC_OPENMP


# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_SYS_LARGEFILE
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset pow setlocale strcspn strrchr])

# Trailer
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/ru/Makefile
                 include/Makefile
		 po/Makefile.in])
AC_OUTPUT
