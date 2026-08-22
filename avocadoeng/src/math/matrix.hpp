#ifndef AVOCADO_MATH_MATRIX
#define AVOCADO_MATH_MATRIX

#include "vecn.hpp"

#include "../core.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

namespace avocado::math {

namespace internal {

template <size_t M, size_t N, typename T>
class Matrix;

template <size_t N, typename T>
vec<T, N> multiply(const Matrix<N, N, T> &matrix, const vec<T, N> &vec);

template <size_t M, size_t N, typename T>
class Matrix {
    static_assert(std::is_floating_point_v<T>, "Type must be the one of floating types");

public:
    constexpr Matrix() = default;

    constexpr Matrix(const std::array<std::array<T, N>, M> &arr) noexcept:
        _arr(arr) {
    }

    constexpr Matrix(std::array<std::array<T, N>, M> &&arr) noexcept :
        _arr(std::move(arr)) {
    }

    /*
     * @brief Convinience constructor for getting matrix from tinygltf.
     */
    [[nodiscard]] static Matrix fromTinyGltf(const std::vector<double> matrix) noexcept
    requires(M == 4 && N == 4) {
        if (!matrix.empty()) {
            return Matrix ({{
                {static_cast<float>(matrix[0]), static_cast<float>(matrix[1]), static_cast<float>(matrix[2]), static_cast<float>(matrix[3])},
                {static_cast<float>(matrix[4]), static_cast<float>(matrix[5]), static_cast<float>(matrix[6]), static_cast<float>(matrix[7])},
                {static_cast<float>(matrix[8]), static_cast<float>(matrix[9]), static_cast<float>(matrix[10]), static_cast<float>(matrix[11])},
                {static_cast<float>(matrix[12]), static_cast<float>(matrix[13]), static_cast<float>(matrix[14]), static_cast<float>(matrix[15])}}});
        }

        return createIdentityMatrix();
    }

    /**
     * @brief Convinience function for getting matrix from tinygltf::Node.
     */
    [[nodiscard]] static Matrix fromSRT(const std::vector<double> &scale,
        const std::vector<double> &rotation, const std::vector<double> &translation)
        requires(M == 4 && N == 4)
    {
        assert((!rotation.empty() || !scale.empty() || !translation.empty()) && "Vectors mustn't be empty");

        const Matrix scaleMatrix = scale.empty() ? createIdentityMatrix() : createScaleMatrix(scale[0], scale[1], scale[2]);
        const Matrix translationMatrix = translation.empty() ? createIdentityMatrix() : createTranslationMatrix(
            translation[0], translation[1], translation[2]);
        const Matrix rotationMatrix = !rotation.empty() ? fromRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]) : createIdentityMatrix();

        return (scaleMatrix * rotationMatrix * translationMatrix);
    }

    static constexpr Matrix createTranslationMatrix(const float x, const float y, const float z)
    requires(M == 4 && N == 4) {
        return Matrix ({{
            {1.f, 0.f, 0.f, 0.f},
            {0.f, 1.f, 0.f, 0.f},
            {0.f, 0.f, 1.f, 0.f},
            { x,   y,   z,  1.f}}});
    }

    static constexpr Matrix createScaleMatrix(const float x, const float y, const float z)
    requires(M == 4 && N == 4) {
        return Matrix ({{
            { x,  0.f, 0.f, 0.f},
            {0.f,  y,  0.f, 0.f},
            {0.f, 0.f,  z,  0.f},
            {0.f, 0.f, 0.f, 1.f}}});
    }

    static constexpr Matrix createIdentityMatrix() noexcept {
        Matrix result;
        for (size_t i = 0; i < N; ++i)
            result[i][i] = T(1);
        return result;
    }

    /**
     * @brief Converts rotation quaternion to rotation matrix.
     *
     * @note Input quaternion must be normalized.
     */
    static Matrix fromRotationQuaternion(float x, float y, float z, float w) noexcept
    requires(M == 4 && N == 4) {
        return Matrix({{
            {1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z,     2 * x * z + 2 * w * y,     0.f},
            {2 * x * y + 2 * w * z,     1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x,     0.f},
            {2 * x * z - 2 * w * y,     2 * y * z + 2 * w * x,     1 - 2 * x * x - 2 * y * y, 0.f},
            {0.f,                       0.f,                       0.f,                       1.f}}});
    }

   bool operator==(const Matrix &other) const {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                if (!avocado::core::areFloatsEq(_arr[i][j], other._arr[i][j])) {
                    return false;
                }
            }
        }

        return true;
    }

    T* operator[](size_t i) noexcept {
        return _arr[i].data();
    }

    const T* operator[](size_t i) const noexcept {
        return _arr[i].data();
    }

    constexpr bool isSquare() const noexcept {
        return (M == N);
    }

    constexpr bool isIdentity() const noexcept {
        if (isSquare()) {
            for (size_t i = 0; i < M; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    if (i == j && !avocado::core::areFloatsEq(_arr[i][j], static_cast<T>(1.0)))
                        return false;

                    if (i != j && !avocado::core::areFloatsEq(_arr[i][j], static_cast<T>(0.0)))
                        return false;
                }
            }
            return true;
        }

        return false;
    }

    constexpr bool isNull() const noexcept {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                if (!avocado::core::areFloatsEq(_arr[i][j], static_cast<T>(0.0)))
                    return false;
            }
        }

        return true;
    }

    constexpr Matrix operator*(const T number) const noexcept {
        Matrix result(*this);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                result[i][j] *= number;
            }
        }

        return result;
    }

    template <size_t K, size_t L>
    constexpr Matrix<N, K, T> operator*(const Matrix<K, L, T> &other) const noexcept
    requires(N == K) {
        Matrix<N, K, T> result;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                for (size_t r = 0; r < N; ++r)
                    result[i][j] += (_arr[i][r] * other[r][j]);
            }
        }

        return result;
    }

    template <size_t K, size_t L>
    constexpr Matrix<N, K, T> operator*=(const Matrix<K, L, T>& other) const noexcept
    requires (N == K) {
        return (*this * other);
    }

    vec<T, M> operator*(const vec<T, M> &v) const {
        return multiply<M, T>(*this, v);
    }

    Matrix operator+(const Matrix &other) const noexcept {
        Matrix <M, N, T> result;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j)
                result[i][j] = _arr[i][j] + other[i][j];
        }

        return result;
    }

    Matrix<M, N, T> operator-(const Matrix<M, N, T> &other) const noexcept {
        Matrix <M, N, T> result;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j)
                result[i][j] = _arr[i][j] - other[i][j];
        }

        return result;
    }

    [[nodiscard]] Matrix<N, M, T> transpose() const noexcept {
        Matrix<N, M, T> result;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                result[j][i] = _arr[i][j];
            }
        }

        return result;
    }

private:
    std::array<std::array<T, N>, M> _arr {};
};

template <size_t M, size_t N, typename T>
Matrix<M, N, T> operator*(const T num, const Matrix<M, N, T> &matrix) {
    return (matrix * num);
}

template <typename T, size_t M, size_t N>
vec<T, M> operator*(const vec<T, M> &v, const Matrix<M, N, T> &matrix) {
    return (matrix * v);
}

template<>
inline vec<float, 2> multiply<2, float>(const Matrix<2, 2, float> &matrix, const vec<float, 2> &vec) {
    return vec2f (
        matrix[0][0] * vec.x + matrix[0][1] * vec.y,
        matrix[1][0] * vec.x + matrix[1][1] * vec.y
    );
}

template<>
inline vec<float, 3> multiply<3, float>(const Matrix<3, 3, float> &matrix, const vec<float, 3> &vec) {
    return vec3f (
        matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z,
        matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z,
        matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z
    );
}

template<>
inline vec<float, 4> multiply<4, float>(const Matrix<4, 4, float> &matrix, const vec<float, 4> &vec) {
    return vec4f (
        matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z + matrix[0][3] * vec.w,
        matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z + matrix[1][3] * vec.w,
        matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z + matrix[2][3] * vec.w,
        matrix[3][0] * vec.x + matrix[3][1] * vec.y + matrix[3][2] * vec.z + matrix[3][3] * vec.w
    );
}

template <size_t N, typename T>
using QuadMatrix = Matrix<N, N, T>;

} // namespace internal.

template<size_t M, size_t N, typename T>
inline std::ostream& operator<<(std::ostream &stream, const internal::Matrix<M, N, T> &matrix) {
    for (size_t i = 0; i < M; ++i) {
        stream << '[';

        for (size_t j = 0; j < N; ++j)
            stream << matrix[i][j] << ' ';

        stream << ']' << std::endl;
    }
    return stream;
}

// todo Do we really need other types except float?
// todo Check for usage of any other types beside Mat4x4 and remove unused ones.
using Mat2x2 = internal::QuadMatrix<2, float>;
using Mat4x4 = internal::QuadMatrix<4, float>;

} // namespace avocado::math.

#endif

