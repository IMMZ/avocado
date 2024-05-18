#ifndef AVOCADO_MATH_FUNCTIONS
#define AVOCADO_MATH_FUNCTIONS

#include "matrix.hpp"
#include "vecn.hpp"

namespace avocado::math {

constexpr float toRadians(const float degrees) {
    return degrees * 0.0174533f;
}

constexpr float toDegrees(const float radians) {
    return radians * 57.2958f;
}

constexpr Mat2x2 createRotationMatrix(const float angle) {
    const float angleRadians = toRadians(angle);
    return Mat2x2({{
        {std::cos(angleRadians), -std::sin(angleRadians)},
        {std::sin(angleRadians), std::cos(angleRadians)}
    }});
}

Mat4x4 createRotationMatrix(const float angleDegrees, vec3f axis);
Mat4x4 createRotationMatrixY(const float angleDegrees);

struct Quaternion;
Mat4x4 createRotationMatrix(Quaternion q);

Mat4x4 lookAt(const vec3f cameraPos, const vec3f targetPos, const vec3f up);

Mat4x4 perspectiveProjection(const float verticalFOV, const float aspectRatio, const float near, const float far);

} // namespace avocado::math.

#endif

