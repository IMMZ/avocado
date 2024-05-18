#include "functions.hpp"

#include "constants.hpp"
#include "quaternion.hpp"

namespace avocado::math {

Mat4x4 createRotationMatrix(const float angleDegrees, vec3f axis) {
    const float angleRadians = toRadians(angleDegrees);
    const float sinA = std::sin(angleRadians);
    const float cosA = std::cos(angleRadians);
    const float oneMinusCosA = (1.f - cosA);

    axis.normalize();
    return Mat4x4({{
        {cosA + oneMinusCosA * axis.x * axis.x,          oneMinusCosA * axis.x * axis.y - sinA * axis.z, oneMinusCosA * axis.x * axis.z + sinA * axis.y, 0.f},
        {oneMinusCosA * axis.x * axis.y + sinA * axis.z, cosA + oneMinusCosA * axis.y * axis.y,          oneMinusCosA * axis.y * axis.z - sinA * axis.x, 0.f},
        {oneMinusCosA * axis.x * axis.z - sinA * axis.y, oneMinusCosA * axis.y * axis.z + sinA * axis.x, cosA + oneMinusCosA * axis.z * axis.z, 0.f},
        {0.f, 0.f, 0.f, 1.f}}});
}

Mat4x4 createRotationMatrixY(const float angleDegrees) {
    const float angleRadians = toRadians(angleDegrees);
    const float sinA = std::sin(angleRadians);
    const float cosA = std::cos(angleRadians);

    return Mat4x4({{
        {cosA, 0.f, sinA, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {-sinA, 0.f, cosA, 0.f},
        {0.f, 0.f, 0.f, 1.f}}});
}

Mat4x4 createRotationMatrix(Quaternion q) {
    q.normalize();
    return Mat4x4({{
        {1.f - 2.f * q.y * q.y - 2.f * q.z * q.z, 2.f * q.x * q.y - 2.f * q.z * q.w,       2.f * q.x * q.z + 2.f * q.y * q.w,       0.f},
        {2.f * q.x * q.y + 2.f * q.z * q.w,       1.f - 2.f * q.x * q.x - 2.f * q.z * q.z, 2.f * q.y * q.z - 2.f * q.x * q.w,       0.f},
        {2.f * q.x * q.z - 2.f * q.y * q.w,       2.f * q.y * q.z + 2.f * q.x * q.w,       1.f - 2.f * q.x * q.x - 2.f * q.y * q.y, 0.f},
        {0.f, 0.f, 0.f, 1.f}}});
}

Mat4x4 lookAt(const vec3f cameraPos, const vec3f targetPos, const vec3f up) {
    vec3f forward = (targetPos - cameraPos); forward.normalize();
    vec3f right = up.crossProduct(forward); right.normalize();
    const vec3f newUp = forward.crossProduct(right);

    return Mat4x4({{
        {right.x,    right.y,    right.z,    -right.dotProduct(cameraPos)},
        {newUp.x,    newUp.y,    newUp.z,    -newUp.dotProduct(cameraPos)},
        {-forward.x, -forward.y, -forward.z, forward.dotProduct(cameraPos)},
        {0.f, 0.f, 0.f, 1.f}}});
}

Mat4x4 perspectiveProjection(const float verticalFOV, const float aspectRatio, const float near, const float far) {
    const float f = 1.0f / tan(toRadians(0.5f * verticalFOV));
    return Mat4x4({{
        {f / aspectRatio, 0.f, 0.f, 0.f},
        {0.f, -f, 0.f, 0.f},
        {0.f, 0.f, far / (near - far), (near * far) / (near - far)},
        {0.f, 0.f, -1.f, 0.f}}});
}

} // namespace avocado::math.

