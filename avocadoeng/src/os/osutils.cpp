#include "osutils.hpp"

namespace avocado::os
{

std::string getExecutablePath() {
#ifdef __linux__
    std::string fullExePath = std::filesystem::canonical("/proc/self/exe").string();
    const std::string::size_type index = fullExePath.find_last_of('/');
    if (std::string::npos != index)
        fullExePath.erase(index, fullExePath.length() - index);

#elif
    static_assert(false, "Implement for Windows");
    return std::string(); // todo implement for Windows.
#endif

    return fullExePath;
}

}

