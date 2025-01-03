#ifndef AVOCADO_OS_UTILS_HPP
#define AVOCADO_OS_UTILS_HPP

#include <filesystem>

namespace avocado::os
{

// todo: from SDL 2.0.1 there's ready function for this.
std::string getExecutablePath();

}

#endif /* ifndef AVOCADO_OS_UTILS_HPP */
