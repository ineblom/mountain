#!/bin/sh

set -eu

FREETYPE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR="$FREETYPE_DIR/build"
LIBRARY="$BUILD_DIR/libfreetype.a"
FREETYPE_CC=${CC:-cc}
FREETYPE_AR=${AR:-ar}

SOURCES="
src/base/ftsystem.c
src/base/ftdebug.c
src/base/ftbase.c
src/base/ftinit.c
src/base/ftbitmap.c
src/base/ftmm.c
src/autofit/autofit.c
src/bdf/bdf.c
src/cff/cff.c
src/cid/type1cid.c
src/pcf/pcf.c
src/pfr/pfr.c
src/psaux/psaux.c
src/pshinter/pshinter.c
src/psnames/psnames.c
src/raster/raster.c
src/sdf/sdf.c
src/sfnt/sfnt.c
src/smooth/smooth.c
src/svg/svg.c
src/truetype/truetype.c
src/type1/type1.c
src/type42/type42.c
src/winfonts/winfnt.c
"

REBUILD=0
if [ ! -f "$LIBRARY" ]; then
  REBUILD=1
elif [ "$FREETYPE_DIR/build.sh" -nt "$LIBRARY" ] || \
     [ "$FREETYPE_DIR/include/freetype/config/ftoption.h" -nt "$LIBRARY" ]; then
  REBUILD=1
else
  for SOURCE in $SOURCES; do
    if [ "$FREETYPE_DIR/$SOURCE" -nt "$LIBRARY" ]; then
      REBUILD=1
      break
    fi
  done
fi

if [ "$REBUILD" = "1" ]; then
  echo "Building vendored FreeType..."
  mkdir -p "$BUILD_DIR"
  OBJECTS=""
  for SOURCE in $SOURCES; do
    OBJECT_NAME=$(printf '%s' "$SOURCE" | tr '/.' '__')
    OBJECT="$BUILD_DIR/$OBJECT_NAME.o"
    "$FREETYPE_CC" -std=c11 -O2 -DNDEBUG -DFT2_BUILD_LIBRARY \
      -I"$FREETYPE_DIR/include" -c "$FREETYPE_DIR/$SOURCE" -o "$OBJECT"
    OBJECTS="$OBJECTS $OBJECT"
  done
  "$FREETYPE_AR" rcs "$LIBRARY" $OBJECTS
fi
