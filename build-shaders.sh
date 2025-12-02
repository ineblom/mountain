#!/bin/sh

glslangValidator -S vert -DGPU_=1 -DGLSL_=1 -V shaders/triangle.vert \
	--glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
	spirv-opt ./shaders/bloat.spv -o ./shaders/triangle.vert.spv --skip-validation
	rm ./shaders/bloat.spv
fi

glslangValidator -S frag -DGPU_=1 -DGLSL_=1 -V shaders/triangle.frag \
	--glsl-version 460 --target-env spirv1.5 -o ./shaders/bloat.spv
if [ $? -eq 0 ]; then
	spirv-opt ./shaders/triangle.frag.spv -o ./shaders/triangle.frag.spv --skip-validation
	rm ./shaders/bloat.spv
fi