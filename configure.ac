#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.66])
AC_INIT([nutrika], [0.2.1])
AC_CONFIG_SRCDIR([src/database.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

PKG_CHECK_MODULES([GTK], [gtk+-2.0])
PKG_CHECK_MODULES([SQLITE], [sqlite3])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

GETTEXT_PACKAGE=nutrika
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Package Name for Gettext])

ALL_LINGUAS="en hu"
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile pixmaps/Makefile po/Makefile.in doc/Makefile])
AC_OUTPUT
