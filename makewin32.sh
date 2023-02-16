#!/bin/sh
touch configure configure.ac aclocal.m4 Makefile.am Makefile.in

./configure --with-win32 \
  'CC=gcc' CFLAGS="-O2 -fcommon"\
  && make \
  && make install-strip prefix=/home/taji/dist32
