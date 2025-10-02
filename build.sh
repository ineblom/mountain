#!/bin/sh

WARN="-Wall -Wno-builtin-requires-header -Wno-incompatible-library-redeclaration -Wno-unused-function -Wno-unused-variable"
DEFS="-DCPU_=1 -DDEV_=1"

cd src
clang $WARN $DEFS -lm -Ofast main.c -o ../m
BUILD_STATUS=$?
cd ..

if [ $BUILD_STATUS -eq 0 ]; then
	echo Build successful, running...
	./m
#	zen output.bmp
else
	echo Build failed
fi
