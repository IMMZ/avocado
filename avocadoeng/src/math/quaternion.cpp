#include "quaternion.hpp"

namespace avocado::math {

void Quaternion::normalize() {
    const float n = norm();
    x /= n;
    y /= n;
    z /= n;
    w /= n;
}

vec3f Quaternion::rotateVector(const vec3f &vecToRotate) const noexcept {
    math::Quaternion q = *this; q.normalize();
    return (q * Quaternion::fromVec4(vecToRotate.toVec4(0.f)) * q.conjugate()).toVec3f();
}

std::ostream& operator<<(std::ostream &stream, const Quaternion &q) {
    stream << "{" << q.w << ", " << q.x << "i, " << q.y << "j, " << q.z << "k}";
    return stream;
}

} // namespace avocado::math.

