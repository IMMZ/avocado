#include "camera.hpp"
#include "math/matrix.hpp"

#include <ostream>

namespace avocado::core {

std::ostream& operator<<(std::ostream &stream, const Camera &camera) {
    using avocado::math::operator<<;
    stream << "[\nperspective:\n" << camera.perspective  << "\nlocalMatrix:\n" << camera.localMatrix << "\nposition: " << camera.position << "\nup: " << camera.up << "\ntargetPosition: " << camera.targetPosition << "]" << std::endl;
    return stream;
}

}
