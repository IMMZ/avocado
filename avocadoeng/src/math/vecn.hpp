#ifndef AVOCADO_CORE_VECN
#define AVOCADO_CORE_VECN

#include "../core.hpp"

#include <array>
#include <cmath>
#include <cstdint>

namespace avocado::math {

namespace internal {

template <typename T, size_t N>
struct vec {
    static_assert(std::is_floating_point_v<T>, "Type must be floating point type.");
    static_assert(N > 1 && N < 5, "N must be equal to 2, 3 or 4.");
};

template <typename T, size_t N>
[[nodiscard]] inline bool isVectorUnit(const vec<T, N> &v) {
    return avocado::core::areFloatsEq(v.length(), static_cast<T>(1.0));
}

template <typename T>
struct vec<T, 2> {
    union {
        T x, w;
    };

    union {
        T y, h;
    };

    constexpr explicit vec(const T a, const T b) noexcept:
        x(a), y(b) {
    }

    [[nodiscard]] bool operator==(const vec other) const {
        return (avocado::core::areFloatsEq(x, other.x) && avocado::core::areFloatsEq(y, other.y));
    }

    [[nodiscard]] constexpr vec operator-() const noexcept {
        return vec(-x, -y);
    }

    [[nodiscard]] constexpr vec operator*(const T n) const noexcept {
        return vec(x * n, y * n);
    }

    [[nodiscard]] constexpr vec operator+(const vec other) const noexcept {
        return vec(x + other.x, y + other.y);
    }

    [[nodiscard]] constexpr vec operator-(const vec other) const noexcept {
        return vec(x - other.x, y - other.y);
    }

    [[nodiscard]] bool isNull() const {
        return (avocado::core::areFloatsEq(x, static_cast<T>(0.0)) && avocado::core::areFloatsEq(y, static_cast<T>(0.0)));
    }

    [[nodiscard]] static constexpr vec createNullVec() noexcept {
        return vec(0.0, 0.0);
    }

    [[nodiscard]] constexpr vec createPerpendicularX() const noexcept {
        return {-y, x};
    }

    [[nodiscard]] constexpr vec createPerpendicularY() const noexcept {
        return {y, -x};
    }

    [[nodiscard]] T length() const {
        return std::sqrt(x * x + y * y);
    }

    [[nodiscard]] constexpr T dotProduct(const vec other) const noexcept {
        return (x * other.x + y * other.y);
    }

    [[nodiscard]] constexpr T scewProduct(const vec other) const noexcept {
        return (x * other.y - other.x * y);
    }

    [[nodiscard]] inline bool isPerpendicularTo(const vec other) const {
        return avocado::core::areFloatsEq(dotProduct(other), 0.0);
    }

    [[nodiscard]] inline bool isUnit() const {
        return isVectorUnit(*this);
    }

    void normalize() {
        const T len = length();
        if (len > T()) {
            x /= len; y /= len;
        }
    }
};

template <typename T>
struct vec<T, 3> {
    union {
        T x, r;
    };

    union {
        T y, g;
    };

    union {
        T z, b;
    };

    constexpr explicit vec(T a, T b, T c) noexcept:
        x(a), y(b), z(c) {
    }

    bool operator==(const vec &other) const {
        return (avocado::core::areFloatsEq(x, other.x)
                && avocado::core::areFloatsEq(y, other.y)
                && avocado::core::areFloatsEq(z, other.z));
    }

    [[nodiscard]] constexpr vec operator-() const noexcept {
        return vec(-x, -y, -z);
    }

    [[nodiscard]] constexpr vec operator+(const vec &other) const noexcept {
        return vec(x + other.x, y + other.y, z + other.z);
    }

    [[nodiscard]] constexpr vec operator-(const vec &other) const noexcept {
        return vec(x - other.x, y - other.y, z - other.z);
    }


    [[nodiscard]] constexpr vec operator*(const T n) const noexcept {
        return vec(x * n, y * n, z * n);
    }

    [[nodiscard]] bool isNull() const {
        return (avocado::core::areFloatsEq(x, static_cast<T>(0.0))
                && avocado::core::areFloatsEq(y, static_cast<T>(0.0))
                && avocado::core::areFloatsEq(z, static_cast<T>(0.0)));
    }

    [[nodiscard]] static constexpr vec createNullVec() noexcept {
        return vec(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0));
    }


    [[nodiscard]] T length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] inline bool isUnit() const {
        return isVectorUnit(*this);
    }

    void normalize() {
        const T len = length();
        if (len > T()) {
            x /= len; y /= len; z /= len;
        }
    }

    [[nodiscard]] constexpr T dotProduct(const vec other) const noexcept {
        return (x * other.x + y * other.y + z * other.z);
    }

    [[nodiscard]] constexpr vec crossProduct(const vec other) const noexcept {
        return vec(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x);
    }
};

template <typename T>
struct vec<T, 4> {
    union {T x, r;};
    union {T y, g;};
    union {T z, b;};
    union {T w, a;};

    constexpr explicit vec(const T i, const T j, const T k, const T l) noexcept:
        x(i), y(j), z(k), w(l) {
    }

    bool operator==(const vec &other) const {
        return (avocado::core::areFloatsEq(x, other.x)
                && avocado::core::areFloatsEq(y, other.y)
                && avocado::core::areFloatsEq(z, other.z)
                && avocado::core::areFloatsEq(w, other.w));
    }

    [[nodiscard]] constexpr vec operator-() const noexcept {
        return vec(-x, -y, -z, -w);
    }

    [[nodiscard]] constexpr vec operator+(const vec &other) const noexcept {
        return vec(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    [[nodiscard]] constexpr vec operator-(const vec &other) const noexcept {
        return vec(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    [[nodiscard]] constexpr vec operator*(const T n) const noexcept {
        return vec(x * n, y * n, z * n, w * n);
    }

    [[nodiscard]] bool isNull() const {
        return (avocado::core::areFloatsEq(x, static_cast<T>(0.0))
                && avocado::core::areFloatsEq(y, static_cast<T>(0.0))
                && avocado::core::areFloatsEq(z, static_cast<T>(0.0))
                && avocado::core::areFloatsEq(w, static_cast<T>(0.0)));
    }

    [[nodiscard]] static constexpr vec createNullVec() noexcept {
        return vec(static_cast<T>(0.0), static_cast<T>(0.0),
                   static_cast<T>(0.0), static_cast<T>(0.0));
    }


    [[nodiscard]] T length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    [[nodiscard]] inline bool isUnit() const {
        return isVectorUnit(*this);
    }

    void normalize() {
        const T len = length();
        if (len > 0.0) {
            x /= len; y /= len; z /= len, w /= len;
        }
    }
};

template <typename T, size_t N>
[[nodiscard]] constexpr vec<T, N> operator*(const T val, const vec<T, N> &vec) {
    return vec * val;
}

} // namespace internal.

using vec2f = internal::vec<float, 2>;
using vec3f = internal::vec<float, 3>;
using vec4f = internal::vec<float, 4>;

} // namespace avocado::math.

#endif

