#! /bin/sh
# vi:ts=4:et
set -e
echo "// Using GNU C compiler."
echo "//"

test "X${top_srcdir}" = X && top_srcdir=`echo "$0" | sed 's,[^/]*$,,'`../..

CC="gcc -fPIC"
CC="gcc -static"
CC="gcc"
CFLAGS="-Wall -Wno-uninitialized -O2 -fomit-frame-pointer"

# for AIX
CC="gcc -maix64"
ARFLAGS="-X64 -rcs"

. $top_srcdir/B/generic/build.sh
