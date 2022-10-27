#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([iarmmgrs-hal], [1.0], [ridish.ra@lnttechservices.com])
AC_CONFIG_SRCDIR([ir/plat-ir.c])

AM_INIT_AUTOMAKE(1.0)

# Checks for programs.
AC_PROG_CC

# Checks for libtool
LT_INIT

# Libs and cflags
IARMMGRS_HAL_IR_LIBS=""
IARMMGRS_HAL_POWER_LIBS=""
IARMMGRS_HAL_DEEPSLEEP_LIBS=""
AC_SUBST(IARMMGRS_HAL_IR_LIBS)
AC_SUBST(IARMMGRS_HAL_POWER_LIBS)
AC_SUBST(IARMMGRS_HAL_DEEPSLEEP_LIBS)

# More libs and cflags should using pkg-config later.
# now we just temperary hard coded here.
IARMMGRS_HAL_IR_CFLAGS="-I$PKG_CONFIG_SYSROOT_DIR/usr/include/rdk/iarmmgrs-hal"
IARMMGRS_HAL_POWER_CFLAGS="-I$PKG_CONFIG_SYSROOT_DIR/usr/include/rdk/iarmmgrs-hal \
                           -I$PKG_CONFIG_SYSROOT_DIR/usr/include/rdk/iarmbus"
IARMMGRS_HAL_DEEPSLEEP_CFLAGS="-I$PKG_CONFIG_SYSROOT_DIR/usr/include/rdk/iarmmgrs-hal \
                           -I$PKG_CONFIG_SYSROOT_DIR/usr/include/rdk/iarmbus"
AC_SUBST(IARMMGRS_HAL_IR_CFLAGS)
AC_SUBST(IARMMGRS_HAL_POWER_CFLAGS)
AC_SUBST(IARMMGRS_HAL_DEEPSLEEP_CFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile ir/Makefile power/Makefile deepsleep/Makefile])
AC_OUTPUT
