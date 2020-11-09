#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([arch/osx/arch.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([readline], [readline])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT8_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([memmove memset select sqrt strdup strndup])

AC_CONFIG_FILES([Makefile
                 t/Makefile])
AC_OUTPUT
