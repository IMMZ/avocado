#include "logger.hpp"

#include "core.hpp"

namespace avocado::core {

void Logger::logInformation(const std::string_view &message) {
    _stream << "[INF] " << message << std::endl;
}

void Logger::logWarning(const std::string_view &message) {
    _stream << "[WAR] " << message << std::endl;
}

void Logger::logError(const std::string_view &message) {
    _stream << "[ERR] " << message << std::endl;
}

void Logger::logDebug(const std::string_view &message) {
    if constexpr (isDebugBuild())
        _stream << "[DBG] " << message << std::endl;
}

Logger& Logger::instance(const Type type, std::ostream &stream) {
    static Logger logger(type, stream);
    return logger;
}

Logger::Logger(const Type type, std::ostream &stream):
    _stream(stream) {}

} // namespace avocado::core
