#include "utils.hpp"

namespace avocado::utils {

std::vector<char> readFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios_base::ate | std::ios_base::binary);
    if (!file.is_open()) {
        return {};
    }

    const size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), buffer.size());
    file.close();
    buffer.shrink_to_fit();

    return buffer;
}

} // namespace avocado::utils.

