#ifndef AVOCADO_CORE_HPP
#define AVOCADO_CORE_HPP

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

} // namespace avocado::core.

#endif
