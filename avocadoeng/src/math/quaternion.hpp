#ifndef AVOCADO_MATH_QUATERNION
#define AVOCADO_MATH_QUATERNION

#include "../core.hpp"

#include <iostream>

namespace avocado::math {

struct Quaternion {
    float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

    [[nodiscard]] bool operator==(const Quaternion &other) const {
        return (avocado::core::areFloatsEq(x, other.x)
                && avocado::core::areFloatsEq(y, other.y)
                && avocado::core::areFloatsEq(z, other.z)
                && avocado::core::areFloatsEq(w, other.w)
                );
    }

    [[nodiscard]] constexpr Quaternion operator+(const Quaternion &other) const noexcept {
        return Quaternion {x + other.x, y + other.y, z + other.z, w + other.w};
    }

    [[nodiscard]] constexpr Quaternion operator-(const Quaternion &other) const noexcept {
        return Quaternion {x - other.x, y - other.y, z - other.z, w - other.w};
    }

    [[nodiscard]] constexpr Quaternion operator*(const Quaternion &other) const noexcept {
        return Quaternion {
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        };
    }

    [[nodiscard]] constexpr Quaternion operator*(const float scalar) const noexcept {
        return Quaternion{x * scalar, y * scalar, z * scalar, w * scalar};
    }

    [[nodiscard]] constexpr Quaternion operator/(const float scalar) const noexcept {
        return Quaternion{x / scalar, y / scalar, z / scalar, w / scalar};
    }

    Quaternion operator/(const Quaternion &other) const {
        const float otherNorm = other.norm();
        if (avocado::core::areFloatsEq(otherNorm, 0.f))
            return Quaternion{};
        return (*this) * other.conjugate() / (otherNorm * otherNorm);
    }

    [[nodiscard]] constexpr static Quaternion createUnit() noexcept {
        return Quaternion{0.f, 0.f, 0.f, 1.f};
    }

    [[nodiscard]] constexpr static Quaternion createReal(const float w) noexcept {
        return Quaternion{0.f, 0.f, 0.f, w};
    }

    [[nodiscard]] constexpr static Quaternion createPure(const float x, const float y, const float z) noexcept {
        return Quaternion{x, y, z, 0.f};
    }

    [[nodiscard]] constexpr Quaternion conjugate() const noexcept {
        return Quaternion{-x, -y, -z, w};
    }

    [[nodiscard]] float norm() const {
        return std::pow(x * x + y * y + z * z + w * w, 0.5f);
    }

    void normalize();
};

std::ostream& operator<<(std::ostream &stream, const Quaternion &q);

} // namespace avocado::math.

#endif

