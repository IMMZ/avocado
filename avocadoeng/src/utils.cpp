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

bool endsWith(const std::string_view &str, const std::string_view &target) {
    if (target.size() > 0 && str.size() >= target.size()) {
        for (size_t i = 0, j = str.size() - target.size(); i < target.size(); ++i, ++j) {
            if (str[j] != target[i])
                return false;
        }

        return true;
    }

    return false;
}

std::vector<std::string> splitString(const std::string &str, const std::string &delimiter, const bool includeEmpty) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        std::string token = str.substr(start, end - start);
        if (includeEmpty || !token.empty()) {
            tokens.push_back(token);
        }
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    // Handle the last token
    std::string token = str.substr(start);
    if (includeEmpty || !token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

} // namespace avocado::utils.

