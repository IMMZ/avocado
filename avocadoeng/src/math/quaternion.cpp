#include "quaternion.hpp"

namespace avocado::math {

void Quaternion::normalize() {
    const float n = norm();
    x /= n;
    y /= n;
    z /= n;
    w /= n;
}

std::ostream& operator<<(std::ostream &stream, const Quaternion &q) {
    stream << "{" << q.w << ", " << q.x << "i, " << q.y << "j, " << q.z << "k}";
    return stream;
}

} // namespace avocado::math.

