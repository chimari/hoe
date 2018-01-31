#!/bin/sh

./configure --with-gtkmac CPPFLAGS=-I/usr/local/opt/openssl/include \
  && make
