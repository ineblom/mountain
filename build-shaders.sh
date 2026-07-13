#!/bin/sh

glslangValidator -S vert -V shaders/shader.vert \
  --glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
  spirv-opt ./shaders/bloat.spv -o ./shaders/shader.vert.spv --skip-validation
  rm ./shaders/bloat.spv
fi

glslangValidator -S frag -V shaders/shader.frag \
  --glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
  spirv-opt ./shaders/bloat.spv -o ./shaders/shader.frag.spv --skip-validation
  rm ./shaders/bloat.spv
fi

glslangValidator -S vert -V shaders/mesh.vert \
  --glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
  spirv-opt ./shaders/bloat.spv -o ./shaders/mesh.vert.spv --skip-validation
  rm ./shaders/bloat.spv
fi

glslangValidator -S frag -V shaders/mesh.frag \
  --glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
  spirv-opt ./shaders/bloat.spv -o ./shaders/mesh.frag.spv --skip-validation
  rm ./shaders/bloat.spv
fi

glslangValidator -S vert -V shaders/mesh_outline.vert \
  --glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
  spirv-opt ./shaders/bloat.spv -o ./shaders/mesh_outline.vert.spv --skip-validation
  rm ./shaders/bloat.spv
fi

glslangValidator -S frag -V shaders/mesh_outline.frag \
  --glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
  spirv-opt ./shaders/bloat.spv -o ./shaders/mesh_outline.frag.spv --skip-validation
  rm ./shaders/bloat.spv
fi
