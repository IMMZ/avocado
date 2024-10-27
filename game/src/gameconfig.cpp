#include "gameconfig.hpp"

#include <utils.hpp>

#include <filesystem>
#include <fstream>

bool GameConfig::load(const std::string_view &filePath) {
    _map.clear();
    _fileName = std::string(filePath);

    if (!std::filesystem::exists(std::filesystem::path(filePath)))
        return false;


    std::ifstream inputFile(_fileName);
    if (!inputFile)
        return false;

    std::string line;
    while (std::getline(inputFile, line, '\n')) {
        if (line.empty() || line.starts_with('#') || !avocado::utils::stringContains(line, "="))
            continue;

        if (line.front() == '=')
            setValue(std::string(), line.substr(1, line.length() - 1));
        else if (line.back() == '=') {
            setValue(line.substr(0, line.length() - 1), std::string());
        } else {
            std::vector tokens = avocado::utils::splitString(line, "=", false);
            if (!tokens.front().empty()) {
                std::string valueToInsert = line.substr(tokens[0].length() + 1, line.length() - tokens[0].length());
                setValue(std::move(tokens[0]), std::move(valueToInsert));
            }
        }
    }
    inputFile.close();
    return true;
}

bool GameConfig::save() {
    std::ofstream outputFile(_fileName, std::ios_base::trunc | std::ios_base::out);
    if (!outputFile)
        return false;

    for (const auto &keyValuePair: _map)
        outputFile << keyValuePair.first << '=' << keyValuePair.second << '\n';

    outputFile.close();
    return true;
}

const std::string& GameConfig::getValue(const std::string &key) const {
    return _map.at(key);
}

bool GameConfig::hasValue(const std::string &key) const {
    return _map.contains(key);
}

void GameConfig::setValue(const std::string &key, const std::string &value) {
    _map[key] = value;
}

void GameConfig::setValue(const std::string &key, std::string &&value) {
    _map[key] = std::move(value);
}

