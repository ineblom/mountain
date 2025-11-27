#!/bin/sh

WARN="-Wall -Wno-builtin-requires-header -Wno-incompatible-library-redeclaration -Wno-unused-function -Wno-unused-variable"
DEFS="-DCPU_=1 -DDEV_=1"
LIBS="-lm -lwayland-client -lwayland-egl -lEGL -lGL"

cd src
clang $WARN $DEFS $LIBS -g -march=native build.c -o ../m
BUILD_STATUS=$?
cd ..

if [ $BUILD_STATUS -eq 0 ]; then
	echo Build successful, running...
	./m
else
	echo Build failed
fi
