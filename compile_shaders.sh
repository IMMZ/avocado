#!/bin/sh

# Fragment shaders.
for file in ./assets/shaders/*.frag
do
    echo "Compiling $file..."
    glslc $file -o $file.spv
done

# Vertex shaders.
for file in ./assets/shaders/*.vert
do
    echo "Compiling $file..."
    glslc $file -o $file.spv
done

exit

