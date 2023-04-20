#ifndef AVOCADO_CORE_HPP
#define AVOCADO_CORE_HPP

#include <cmath>
#include <limits>

namespace avocado::core {

constexpr bool isDebugBuild() {
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

constexpr bool isReleaseBuild() {
    return !isDebugBuild();
}

namespace internal {
    template<typename T>
    constexpr T DefaultEpsilonValue = 0.00001f;
    template<>
    inline constexpr double DefaultEpsilonValue<double> = 0.00001;
    template<>
    inline constexpr long double DefaultEpsilonValue<long double> = 0.00001l;
}

template <typename F>
bool areFloatsEq(const F a, const F b, const F epsilon = internal::DefaultEpsilonValue<F>) {
    static_assert(std::is_floating_point_v<F>, "Type must be a floating point type");

    // todo constexpr since C++23.
    return std::fabs(a - b) < epsilon;
}

} // namespace avocado::core.

#endif
