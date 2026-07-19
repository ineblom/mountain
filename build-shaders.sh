#!/bin/sh

set -eu

if ! command -v glslangValidator >/dev/null 2>&1; then
  echo "glslangValidator was not found on PATH." >&2
  exit 1
fi
if ! command -v spirv-opt >/dev/null 2>&1; then
  echo "spirv-opt was not found on PATH." >&2
  exit 1
fi

TEMP_SPV="shaders/.mountain-shader.tmp.spv"
trap 'rm -f "$TEMP_SPV"' EXIT HUP INT TERM

for SOURCE_PATH in shaders/*.vert shaders/*.frag; do
  STAGE="${SOURCE_PATH##*.}"
  OUTPUT_PATH="$SOURCE_PATH.spv"
  echo "Compiling $SOURCE_PATH"
  glslangValidator -S "$STAGE" -V "$SOURCE_PATH" \
    --glsl-version 460 --target-env spirv1.5 -o "$TEMP_SPV"
  spirv-opt "$TEMP_SPV" -o "$OUTPUT_PATH"
done
