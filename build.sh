#!/bin/sh

WARN="-Wall -Wno-unused-function -Wno-unused-variable"
DEFS="-DDEV=1"
OPT="-fno-omit-frame-pointer"

case "$(uname -s)" in
  Darwin)
    COMPILER="${CC:-clang}"
    LANGUAGE="-x objective-c"
    LANGUAGE_END="-x none"
    LIB="-lm -framework Cocoa -framework QuartzCore"
    INC="-I../deps/freetype/include -I../deps/vulkan/include -I../deps/wayland"

    if [ -n "$VULKAN_SDK" ] && [ -d "$VULKAN_SDK/lib" ]; then
      LIB="-L$VULKAN_SDK/lib -Wl,-rpath,$VULKAN_SDK/lib $LIB"
    fi
    ;;
  *)
    COMPILER="${CC:-gcc}"
    LANGUAGE=""
    LANGUAGE_END=""
    INC="-I../deps/freetype/include -I../deps/vulkan/include -I../deps/wayland"
    LIB="-lm -lwayland-client -lwayland-cursor -lxkbcommon"
    OPT="$OPT -maes -mssse3"
    ;;
esac

./deps/freetype/build.sh || exit $?
LIB="../deps/freetype/build/libfreetype.a $LIB"

echo Building...

SHADERS_DIRTY=0
for SHADER_SOURCE in shaders/*.vert shaders/*.frag; do
  if [ ! -f "$SHADER_SOURCE.spv" ] || [ "$SHADER_SOURCE" -nt "$SHADER_SOURCE.spv" ]; then
    SHADERS_DIRTY=1
    break
  fi
done
if [ $SHADERS_DIRTY -eq 1 ]; then
  echo "Building shaders..."
  ./build-shaders.sh || exit $?
fi

if [ "$1" = "META" ]; then
  cd src || exit 1
  $COMPILER $LANGUAGE $WARN $DEFS $INC $OPT -DMETA_APP=1 build.c -o ../meta $LANGUAGE_END $LIB
  BUILD_STATUS=$?
  cd .. || exit 1

  if [ $BUILD_STATUS -eq 0 ]; then
    echo "META build successful."
    ./meta
  else
    echo "META build failed."
    exit $BUILD_STATUS
  fi
fi

cd src || exit 1
$COMPILER $LANGUAGE $WARN $DEFS $INC $OPT build.c -o ../m $LANGUAGE_END $LIB
BUILD_STATUS=$?
cd .. || exit 1

if [ $BUILD_STATUS -eq 0 ]; then
  echo "Build successful."
  if [ "${BUILD_ONLY:-0}" != "1" ]; then
    ./m
  fi
else
  echo "Build failed."
  exit $BUILD_STATUS
fi
