#!/bin/sh

set -e
# optional: amd64 or arm64
ARCH="amd64"
DEBUG_FLAGS="-g -O0"
RELEASE_FLAGS="-O3"
CXFLAGS="-Werror -Wall -std=c++11 -fPIC"
CHECK_FLAGS="-fsanitize=address"
STATIC_LIBS="3rdparty/libuvc/libs/$ARCH/libuvc.a"
LIB_DIRS="-L."
LIBS="-lopencv_core -lopencv_imgcodecs -lopencv_highgui -l:libcamera.a -ludev"
SRC="src/*.cc"
TEST_SRC="test/src/*.cc"
INCLUDE_DIRS="-Isrc -I3rdparty/libuvc/include -I/usr/include/opencv4"
TARGET_NAME="libcamera.a"
# DEBUG
# g++ -c $SRC $INCLUDE_DIRS $CXFLAGS $DEBUG_FLAGS $CHECK_FLAGS
# ar -x $STATIC_LIBS
# ar -cr $TARGET_NAME *.o
# g++ $TEST_SRC $INCLUDE_DIRS $CXFLAGS $DEBUG_FLAGS $CHECK_FLAGS $LIB_DIRS $LIBS -o test_bin

# RELEASE
g++ -c $SRC $INCLUDE_DIRS $CXFLAGS $RELEASE_FLAGS
ar -x $STATIC_LIBS
ar -cr $TARGET_NAME *.o

sh clean.sh
