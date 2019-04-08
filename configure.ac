#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([Osmo], [0.2.10], [wdlkmpx@github])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign])
AC_CONFIG_SRCDIR([src/contacts.c])
AC_CONFIG_HEADER([config.h])
AC_MSG_CHECKING(for type of build)
PKG_PROG_PKG_CONFIG
LT_INIT

# Checks for enabled modules

AC_ARG_WITH(contacts,
	[[  --without-contacts      disable Contacts module]],
	[if test "x$withval" = "xyes" ; then contacts="yes" ; else contacts="no"; fi],
	contacts="yes")

AC_ARG_WITH(tasks,
	[[  --without-tasks         disable Tasks module]],
	[if test "x$withval" = "xyes" ; then tasks="yes" ; else tasks="no"; fi],
	tasks="yes")

AC_ARG_WITH(notes,
	[[  --without-notes         disable Notes module]],
	[if test "x$withval" = "xyes" ; then notes="yes" ; else notes="no"; fi],
	notes="yes")
if test "x$notes" = "xyes"; then
	AC_DEFINE([NOTES_ENABLED], [1], [Notes module enabled])
fi

# Checks for programs.
SAVEDFLAGS=${CFLAGS}
AC_PROG_CC
CFLAGS=${SAVEDFLAGS}

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h locale.h limits.h fcntl.h stdlib.h string.h sys/time.h unistd.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([floor memset setlocale strncasecmp strstr strtol])

# Checks for msgfmt
AC_PATH_PROG(MSGFMT, msgfmt, msgfmt)

# Checks for libraries.
AM_PATH_GTK_2_0(2.12.0,,
	AC_MSG_ERROR([GTK+ not found or too old (version < 2.12)]))

AM_PATH_XML2(2.0.0,,
	AC_MSG_ERROR([You do not appear to have libxml2 installed.]))

PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.6.0)

# Checks for libtar
AC_CHECK_LIB(tar, tar_append_file, [lib=yes], [lib=no])
if test "x$lib" = "xyes"; then
	libtar="yes"
	tar_LIBS="-ltar"
	AC_DEFINE([HAVE_LIBTAR], [1], [Defined to 1 if compile with libtar support])
else
	libtar="no"
fi

# Checks for libgringotts
AC_CHECK_LIB(gringotts, grg_context_initialize_defaults, [lib=yes], [lib=no])
libgringotts="no"
if test "$lib" = "yes"; then
	libgringotts="yes"
fi
if test "x$lib" = "xyes"; then
	gringotts_LIBS="-lgringotts"
	AC_DEFINE([HAVE_LIBGRINGOTTS], [1], [Defined to 1 if compile with libgringotts support])
fi

# Checks for libical
PKG_CHECK_MODULES(LIBICAL, libical >= 0.33, have_libical=true,
	have_libical=false)
if test "x$have_libical" = "xtrue"; then
	AC_DEFINE([HAVE_LIBICAL], [1], [Definded to 1 if compile with libical support])
	libical="yes"
else
	libical="no"
fi
AM_CONDITIONAL([HAVE_LIBICAL], test "x$have_libical" = "xtrue")

# Checks for gtkspell
PKG_CHECK_MODULES(GTKSPELL, gtkspell-2.0 >= 2.0.5, have_gtkspell=true,
	have_gtkspell=false)
if test "x$have_gtkspell" = "xtrue"; then
	AC_DEFINE([HAVE_GTKSPELL], [1], [Definded to 1 if compile with gtkspell support])
	gtkspell="yes"
else
	gtkspell="no"
fi
AM_CONDITIONAL([HAVE_GTKSPELL], test "x$have_gtkspell" = "xtrue")

# Checks for libnotify
PKG_CHECK_MODULES(LIBNOTIFY, libnotify >= 0.4.4, have_libnotify=true,
	have_libnotify=false)
if test "x$have_libnotify" = "xtrue"; then
	AC_DEFINE([HAVE_LIBNOTIFY], [1], [Definded to 1 if compile with libnotify support])
	libnotify="yes"
else
	libnotify="no"
fi
AM_CONDITIONAL([TASKS_ENABLED], test "x$tasks" = "xyes")
AM_CONDITIONAL([HAVE_LIBNOTIFY], test "x$have_libnotify" = "xtrue")

if test "x$tasks" = "xyes"; then
	AC_DEFINE([TASKS_ENABLED], [1], [Tasks module enabled])
fi

# Checks for libgtkhtml
PKG_CHECK_MODULES(LIBGTKHTML, libgtkhtml-2.0 >= 2.2.0, have_libgtkhtml=true,
	have_libgtkhtml=false)
if test "x$have_libgtkhtml" = "xtrue"; then
	AC_DEFINE([HAVE_LIBGTKHTML], [1], [Definded to 1 if compile with libgtkhtml support])
	libgtkhtml="yes"
else
	libgtkhtml="no"
fi
AM_CONDITIONAL([HAVE_LIBGTKHTML], test "x$have_libgtkhtml" = "xtrue")
if test "x$have_libgtkhtml" = "xfalse"; then contacts="no"; fi

if test "x$contacts" = "xyes"; then
	AC_DEFINE([CONTACTS_ENABLED], [1], [Contacts module enabled])
fi

# Checks for Pango 1.20
PKG_CHECK_MODULES(PANGO_1_20, pango >= 1.20, [HAVE_PANGO_1_20=yes], [HAVE_PANGO_1_20=no])
if test "x$HAVE_PANGO_1_20" = xyes; then
	AC_DEFINE(HAVE_PANGO_1_20, 1, [Define to 1 if you have Pango-1.20])
fi

# Checks for additional options

AC_ARG_ENABLE(
	debug,
	[  --enable-debug=yes,no   compile with debugging support (default: no)],
	debug="$enableval",
	debug="no")
if test "x$debug" = "xyes"; then
	BUILD_CFLAGS="-rdynamic -ggdb -g -O0"
	AC_DEFINE([DEBUG_BUILD], [1], [Defined if doing a debug build])
fi

AC_ARG_ENABLE(
	printing,
	[  --enable-printing=yes,no   compile with printing support (default: yes)],
	printing="$enableval",
	printing="yes")
if test "x$printing" = "xyes"; then
	AC_DEFINE([PRINTING_SUPPORT], [1], [Defined if printing support is enabled])
fi

AC_ARG_ENABLE(
	backup,
	[  --enable-backup=yes,no   compile with backup support (default: yes)],
	backup="$enableval",
	backup="yes")
if test "x$backup" = "xyes"; then
	if test "x$libtar" = "xyes"; then
		if test "x$libgringotts" = "xyes"; then
			AC_DEFINE([BACKUP_SUPPORT], [1], [Defined if backup support is enabled])
		else
			backup="no"
		fi
	else
		backup="no"
	fi
fi

AC_ARG_WITH([config-path],
			AS_HELP_STRING([--with-config-path=PATH], [use a non-default config path]),
			[configpath=$withval AC_SUBST([configpath])])
AM_CONDITIONAL([CONFIGPATH], [test x$configpath != x])
AC_ARG_WITH([config-dir],
			AS_HELP_STRING([--with-config-dir=DIR], [use a non-default config directory]),
			[configdir=$withval AC_SUBST([configdir])])
AM_CONDITIONAL([CONFIGDIR], [test x$configdir != x])

CFLAGS=${CFLAGS:="$BUILD_CFLAGS -Wall $PLATFORM_CFLAGS -D_GNU_SOURCE"}
CXXFLAGS=${CXXFLAGS:="$CFLAGS"}
CPPFLAGS=""
LIBS="$tar_LIBS $gringotts_LIBS"

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	po/Makefile
	data/Makefile
	data/icons/Makefile
	data/icons/16x16/Makefile
	data/icons/22x22/Makefile
	data/icons/24x24/Makefile
	data/icons/32x32/Makefile
	data/icons/48x48/Makefile
	data/icons/scalable/Makefile
	data/pixmaps/Makefile
	data/sounds/Makefile
])
AC_OUTPUT

# WIN32 build
AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")

echo "
Configuration:

    Prefix:             $prefix
    Debug enabled:      $debug

    Backup support:     $backup (libgringotts and libtar are required)
    Printing support:   $printing

Enabled modules:

    Calendar:           yes (always)
    Tasks:              $tasks
    Contacts:           $contacts (libgtkhtml is required)
    Notes:              $notes

Optional features:

    gtkspell:           $gtkspell
    libical:            $libical
    libnotify:          $libnotify
    libgtkhtml:         $libgtkhtml
    libtar:             $libtar
    libgringotts:       $libgringotts
"
