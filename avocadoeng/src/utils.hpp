#ifndef AVOCADO_UTILS
#define AVOCADO_UTILS

#include <algorithm>
#include <fstream>
#include <type_traits>
#include <vector>

#define MAKE_COPYABLE(classname)\
    classname(const classname&) = default;\
    classname& operator=(const classname&) = default

#define MAKE_MOVABLE(classname)\
    classname(classname&&) = default;\
    classname& operator=(classname&&) = default

#define NON_COPYABLE(classname)\
    classname(const classname&) = delete;\
    classname& operator=(const classname&) = delete

#define NON_MOVABLE(classname)\
    classname(classname&&) = delete;\
    classname& operator=(classname&&) = delete

namespace avocado::utils {

template<typename T, typename ...Ts>
constexpr bool areOfSameType() {
    return std::conjunction_v<std::is_same<T, Ts>...>;
}

template<typename ...Enums>
constexpr std::common_type_t<Enums...> enumBitwiseOr(Enums ...enums) {
    static_assert(std::conjunction_v<std::is_enum<Enums>...>, "All types must be of enum type.");
    static_assert(areOfSameType<Enums...>(), "All types must be the same.");
    return static_cast<std::common_type_t<Enums...>>((static_cast<std::underlying_type_t<Enums>>(enums) | ...));
}

std::vector<char> readFile(const std::string &filePath);

inline bool areFloatEq(const float a, const float b) {
    return fabs(a - b) < std::numeric_limits<float>::epsilon();
}

template <typename Container>
void makeUniqueContainer(Container &cont) {
    std::sort(cont.begin(), cont.end());
    const auto newEnd = std::unique(cont.begin(), cont.end());
    cont.erase(newEnd, cont.end());
}

} // namespace avocado::utils.

#endif

