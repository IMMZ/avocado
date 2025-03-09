#include "vertex.hpp"

namespace avocado {

std::ostream& operator<<(std::ostream &stream, const Vertex &vertex) {
    stream << "{" << vertex.position << ", " << vertex.color << ", " << vertex.textureCoordinate << "}";
    return stream;
}

} // namespace avocado.

