#!/bin/sh

WARN="-Wall -Wno-unused-function -Wno-unused-variable"
DEFS="-DCPU_=1 -DDEV_=1"
INC="-I/usr/include/freetype2"
LIB="-lm -lwayland-client -lwayland-cursor -lfreetype"

cd src
clang $WARN $DEFS $INC $LIB -g -march=native build.c -o ../m
BUILD_STATUS=$?
cd ..

if [ $BUILD_STATUS -eq 0 ]; then
	echo Build successful, running...
	./m
else
	echo Build failed
fi
