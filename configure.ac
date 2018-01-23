dnl Process this file with autoconf to produce a configure script.
AC_INIT(main.c)
PACKAGE=hoe

dnl version number
MAJOR_VERSION=2
MINOR_VERSION=7
MICRO_VERSION=0
BUILD_REVISION=0
EXTRA_VERSION=
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION$EXTRA_VERSION

dnl set $target
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
dnl AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
dnl AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(BUILD_REVISION)


AM_CONFIG_HEADER(config.h)
AM_PROG_LIBTOOL
dnl AC_ARG_PROGRAM
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_CHECK_PROG(WINDRES, windres, windres)
AM_CONDITIONAL(HAVE_WINDRES, test x"$WINDRES" != x)


# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([pwd.h sys/utsname.h])


dnl Win32
AC_ARG_WITH(win32,
		 AC_HELP_STRING([--with-win32],
		            [Make for Win32]),
			    [use_win32=yes])
if test x"${use_win32}" = "xyes"; then
AC_DEFINE(USE_WIN32,[], [Make for Win32])
CFLAGS="$CFLAGS -mms-bitfields"
LDFLAGS="$LDFLAGS -mwindows"
WINMM_LIBS="-lwinmm"
WINSOCK_LIBS="-lws2_32"
else
WINMM_LIBS=""
WINSOCK_LIBS=""
fi
AC_SUBST(WINMM_LIBS)
AC_SUBST(WINSOCK_LIBS)

dnl Gtk+
dnl AC_ARG_WITH(gtk1,
dnl 		 AC_HELP_STRING([--with-gtk1],
dnl		            [use Gtk+1.2 instead of Gtk+2]),
dnl			    [use_gtk1=yes])
             

dnl if test x"${use_gtk1}" = "xyes"; then
dnl AM_PATH_GLIB(1.2.6,,
dnl         AC_MSG_ERROR(Test for GLIB failed. See the file 'INSTALL' for help.),
dnl         gthread)
dnl AM_PATH_GTK(1.2.0, ,
dnl             AC_MSG_ERROR(Cannot find GTK: Is gtk-config in path?))
dnl else
AM_PATH_GLIB_2_0(2.8.0,, AC_MSG_ERROR([glib-2.x is required]), gthread gobject)
AM_PATH_GTK_2_0(2.8.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK2],[],[Use Gtk+2.x instead of Gtk+1.2])
dnl fi
AC_SEARCH_LIBS([cos], [m], [], [AC_MSG_ERROR([unable to find the cos() function])])



dnl Maintainer Mode
AC_ARG_WITH(maintain,
		 AC_HELP_STRING([--with-maintain],
		            [For Maintainer]),
			    [use_maint=yes])
             

if test x"${use_maint}" = "xyes"; then
	DISABLE_DEPRECATED="-g -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
else
	DISABLE_DEPRECATED=""
fi
AC_SUBST(DISABLE_DEPRECATED)

dnl Check for OpenSSL
AC_ARG_ENABLE(ssl,
	[  --disable-ssl           Disable SSL support using OpenSSL],
	[ac_cv_enable_ssl=$enableval], [ac_cv_enable_ssl=yes])
AC_MSG_CHECKING([whether to use OpenSSL])
if test $ac_cv_enable_ssl = yes; then
	AC_MSG_RESULT(yes)
	dnl First check with pkg-config
	PKG_CHECK_MODULES(OPENSSL, openssl, [:], [:])
	if test "x$OPENSSL_LIBS" != "x"; then
		CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
		LIBS="$LIBS $OPENSSL_LIBS"
		AC_MSG_CHECKING([if openssl is available])
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_SSL, 1, Define if you use OpenSSL to support SSL.)
	else
                AC_MSG_ERROR([OpenSSL is required. Configure with --disable-ssl if you do not need HTTPS access.])
	fi
else
	AC_MSG_RESULT(no)
fi


AC_OUTPUT([
Makefile
version.h
hoe_res.rc
libnova/Makefile
])


dnl Output the configuration summary
echo ""
echo "####################"
if test x"${use_win32}" = "xyes"; then
echo " $PACKAGE $VERSION build for Win32 "
else
echo " $PACKAGE $VERSION"
fi
echo "####################"
echo ""
echo ""
echo "Configure finished, type 'make' to build."