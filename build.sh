#!/bin/sh

WARN="-Wall -Wtype-limits -Wno-unused-function -Wno-unused-variable"
DEFS="-DDEV=1"
OPT="-O0"

META=0
NATIVE=0
SIMD_WIDTH=4
for ARG in "$@"; do
  case "$ARG" in
    release) OPT="-O2" ;;
    native) NATIVE=1 ;;
    simd=4|simd=8|simd=16) SIMD_WIDTH="${ARG#simd=}" ;;
    META) META=1 ;;
    *)
      echo "Usage: $0 [META] [release] [native] [simd=4|8|16]" >&2
      exit 1
      ;;
  esac
done

DEFS="$DEFS -DSIMD_WIDTH=$SIMD_WIDTH"
if [ "$NATIVE" -eq 1 ]; then
  OPT="$OPT -march=native"
fi

# Wider SIMD lanes need the matching instruction set on x86, otherwise the
# compiler falls back to scalar emulation of the vector types.
SIMD_FLAGS=""
case "$(uname -m)" in x86_64|amd64)
  case "$SIMD_WIDTH" in
    8)  SIMD_FLAGS="-mavx2 -mfma" ;;
    16) SIMD_FLAGS="-mavx512f -mavx512dq -mavx512vl" ;;
  esac
  ;;
esac

case "$(uname -s)" in Darwin)
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
OPT="$OPT $SIMD_FLAGS"

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

if [ "$META" -eq 1 ]; then
  cd src || exit 1
  $COMPILER $LANGUAGE $WARN $DEFS $INC $OPT -DMETA_APP=1 build.c -o ../meta $LANGUAGE_END $LIB
  BUILD_STATUS=$?
  cd .. || exit 1

  if [ $BUILD_STATUS -eq 0 ]; then
    echo "META build successful."
    ./meta || exit $?
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
else
  echo "Build failed."
  exit $BUILD_STATUS
fi
