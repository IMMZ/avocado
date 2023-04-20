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
        {cosA + oneMinusCosA * axis.x * axis.x,          oneMinusCosA * axis.x * axis.y - sinA * axis.z, oneMinusCosA * axis.x * axis.z + sinA * axis.y},
        {oneMinusCosA * axis.x * axis.y + sinA * axis.z, cosA + oneMinusCosA * axis.y * axis.y,          oneMinusCosA * axis.y * axis.z - sinA * axis.x},
        {oneMinusCosA * axis.x * axis.z - sinA * axis.y, oneMinusCosA * axis.y * axis.z + sinA * axis.x, cosA + oneMinusCosA * axis.z * axis.z},
        {0.f, 0.f, 0.f, 1.f}
    }});
}

Mat4x4 createRotationMatrix(Quaternion q) {
    q.normalize();
    return Mat4x4({{
        {1.f - 2.f * q.y * q.y - 2.f * q.z * q.z, 2.f * q.x * q.y - 2.f * q.z * q.w,       2.f * q.x * q.z + 2.f * q.y * q.w,       0.f},
        {2.f * q.x * q.y + 2.f * q.z * q.w,       1.f - 2.f * q.x * q.x - 2.f * q.z * q.z, 2.f * q.y * q.z - 2.f * q.x * q.w,       0.f},
        {2.f * q.x * q.z - 2.f * q.y * q.w,       2.f * q.y * q.z + 2.f * q.x * q.w,       1.f - 2.f * q.x * q.x - 2.f * q.y * q.y, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    }});
}

Mat4x4 lookAt(const vec3f cameraPos, const vec3f targetPos, const vec3f up) {
    vec3f forward = (targetPos - cameraPos); forward.normalize();
    vec3f right = forward.crossProduct(up); right.normalize();
    const vec3f newUp = right.crossProduct(forward);

    return Mat4x4({{
        {right.x, newUp.x, -forward.x, 0.f},
        {right.y, newUp.y, -forward.y, 0.f},
        {right.z, newUp.z, -forward.z, 0.f},
        {-right.dotProduct(cameraPos), -newUp.dotProduct(cameraPos), forward.dotProduct(cameraPos), 1.f}
        }});
}

Mat4x4 perspectiveProjection(const float verticalFOV, const float aspectRatio, const float near, const float far) {
    const float focalLenght = 1.f / std::tan((verticalFOV * 2.f * PI / 360.f) / 2.f);
    return Mat4x4({{
        {focalLenght / aspectRatio, 0.f, 0.f, 0.f},
        {0.f, -focalLenght, 0.f, 0.f},
        {0.f, 0.f, near / (far - near), (far * near) / (far - near)},
        {0.f, 0.f, -1.f, 0.f}
    }});
}

} // namespace avocado::math.

