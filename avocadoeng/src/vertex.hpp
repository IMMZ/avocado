#ifndef AVOCADO_VERTEX
#define AVOCADO_VERTEX

#include "math/vecn.hpp"

namespace avocado {

struct Vertex {
    math::vec3f position;
    math::vec3f color;
    math::vec2f textureCoordinate;
};

}

#endif

