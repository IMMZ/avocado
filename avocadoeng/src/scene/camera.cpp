#include "camera.hpp"

namespace avocado::core {

std::ostream& operator<<(std::ostream &stream, const Camera &camera) {
    stream << "[position: " << camera.position << ", up: " << camera.up << "]" << std::endl;
    return stream;
}

}
