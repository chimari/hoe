dnl Process this file with autoconf to produce a configure script.
AC_INIT(main.c)
PACKAGE=hoe

dnl version number
MAJOR_VERSION=6
MINOR_VERSION=0
MICRO_VERSION=3
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


AC_CONFIG_HEADERS([config.h libssh2_config.h])
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
AC_CHECK_HEADERS(error.h,AC_DEFINE(HAVE_ERROR_H,1))
AC_CHECK_HEADERS([pwd.h sys/utsname.h])
AC_CHECK_HEADERS([errno.h fcntl.h stdio.h stdlib.h unistd.h sys/uio.h])
AC_CHECK_HEADERS([sys/select.h sys/socket.h sys/ioctl.h sys/time.h])
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h])
AC_CHECK_HEADERS([sys/un.h], [have_sys_un_h=yes], [have_sys_un_h=no])
AM_CONDITIONAL([HAVE_SYS_UN_H], test "x$have_sys_un_h" = xyes)

os_osx=no

case "$target" in
*-darwin*)
	os_osx=yes
        AC_DEFINE([USE_OSX],[],[Use Mac OS X])
dnl	CFLAGS="$CFLAGS -no-cpp-precomp -fno-common"
	;;
esac

if test x"${os_osx}" = "xyes"; then
        OSX_LDADD="-headerpad_max_install_names"
else
        OSX_LDADD=""
fi
AC_SUBST(OSX_LDADD)

dnl Win32
AC_ARG_WITH(win32,
		 AC_HELP_STRING([--with-win32],
		            [Make for Win32]),
			    [use_win32=yes])
if test x"${use_win32}" = "xyes"; then
AC_DEFINE(USE_WIN32,[], [Make for Win32])
CFLAGS="$CFLAGS -mms-bitfields -DWINVER=0x0501"
LDFLAGS="$LDFLAGS -mwindows"
WINMM_LIBS="-lwinmm"
WINSOCK_LIBS="-lws2_32"
localedir='share/locale'
else
WINMM_LIBS=""
WINSOCK_LIBS=""
localedir='${prefix}/${DATADIRNAME}/locale'
fi
AC_SUBST(localedir)
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
AM_PATH_GLIB_2_0(2.32.0,, AC_MSG_ERROR([glib-2.x is required]), gthread gobject)

dnl GTK 3 or 2 
AC_ARG_WITH(gtk3,
             AC_HELP_STRING([--with-gtk3],
                        [use Gtk+3 instead of Gtk+2]),
                        [use_gtk3=yes])
if test x"${use_gtk3}" = "xyes"; then
GTK3_CFLAGS="`pkg-config gtk+-3.0 --cflags`"
GTK3_LIBS="`pkg-config gtk+-3.0 --libs`"
dnl    AM_PATH_GTK_3_0(3.22.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK3],[],[Use Gtk+3.x instead of Gtk+2])
else
AM_PATH_GTK_2_0(2.24.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK2],[],[Use Gtk+2.x instead of Gtk+3])
GTK3_CFLAGS=""
GTK3_LIBS=""
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)


dnl fi
AC_SEARCH_LIBS([cos], [m], [], [AC_MSG_ERROR([unable to find the cos() function])])

dnl gresource
AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources])

dnl GtkMACIntegration
 AC_ARG_WITH(gtkmac,
 		 AC_HELP_STRING([--with-gtkmac],
		            [make with gtkmacintegration]),
			    [with_gtkmac=yes])
 
 if test x"${with_gtkmac}" = "xyes"; then
   if test "$os_osx" == yes; then
     AC_DEFINE([USE_GTKMACINTEGRATION],[],[With gtkmacintegration])
     GTKMAC_CFLAGS="`pkg-config gtk-mac-integration-gtk2 --cflags`"
     GTKMAC_LIBS="`pkg-config gtk-mac-integration-gtk2 --libs`"
   else
     GTKMAC_CFALGS=""
     GTKMAC_LIBS=""
   fi
 else
   GTKMAC_CFALGS=""
   GTKMAC_LIBS=""
 fi
 AC_SUBST(GTKMAC_LIBS)
 AC_SUBST(GTKMAC_CFLAGS)


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
	PKG_CHECK_MODULES(OPENSSL, openssl, [:], [:])
	if test "x$OPENSSL_LIBS" != "x"; then
		CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
		LIBS="$LIBS $OPENSSL_LIBS"
		AC_MSG_CHECKING([if openssl is available])
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_SSL, 1, Define if you use OpenSSL to support SSL.)
	else
                AC_MSG_ERROR([OpenSSL is required.])
	fi

dnl Check for libssh2
	PKG_CHECK_MODULES(LIBSSH2, libssh2, [:], [:])
	if test "x$LIBSSH2_LIBS" != "x"; then
		CFLAGS="$CFLAGS $LIBSSH2_CFLAGS"
		LIBS="$LIBS $LIBSSH2_LIBS"
		AC_MSG_CHECKING([if libssh2 is available])
		AC_MSG_RESULT(yes)
	else
                AC_MSG_ERROR([LibSSH2 is required.])
	fi

AC_ARG_WITH(libgcrypt,
  AC_HELP_STRING([--with-libgcrypt],[Use libgcrypt for crypto]),
  use_libgcrypt=$withval,use_libgcrypt=auto)
AC_ARG_WITH(wincng,
  AC_HELP_STRING([--with-wincng],[Use Windows CNG for crypto]),
  use_wincng=$withval,use_wincng=auto)
AC_ARG_WITH(libz,
  AC_HELP_STRING([--with-libz],[Use zlib for compression]),
  use_libz=$withval,use_libz=auto)


dnl XML2
dnl AM_PATH_XML2(2.2.6,, AC_MSG_ERROR([xml-2.2.6 or later is required]))
PKG_CHECK_MODULES([XML2], [libxml-2.0])
if test "x$XML2_LIBS" != "x"; then
	CFLAGS="$CFLAGS $XML2_CFLAGS"
	LIBS="$LIBS $XML2_LIBS"
	AC_MSG_CHECKING([if libxml2 is available])
	AC_MSG_RESULT(yes)
else
        AC_MSG_ERROR([libxml2 is required.])
fi

dnl json-c
PKG_CHECK_MODULES(JSONC, json-c, [:], [:])
if test "x$JSONC_LIBS" != "x"; then
	CFLAGS="$CFLAGS $JSONC_CFLAGS"
	LIBS="$LIBS $JSONC_LIBS"
	AC_MSG_CHECKING([if json-c is available])
	AC_MSG_RESULT(yes)
else
        AC_MSG_ERROR([json-c is required.])
fi



AC_OUTPUT([
Makefile
version.h
hoe_res.rc
libnova/Makefile
icons/Makefile
debian/Makefile
ini/Makefile
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
