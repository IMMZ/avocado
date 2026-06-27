#ifndef AVOCADO_CORE_LOGGER
#define AVOCADO_CORE_LOGGER

#include "core.hpp"

#include <iostream>

namespace avocado::core {

class Logger {
public:
    enum class Type { Console };

    DISABLE_COPY_AND_MOVE(Logger);

    void logInformation(const std::string_view &message);
    void logWarning(const std::string_view &message);
    void logError(const std::string_view &message);
    void logDebug(const std::string_view &message);

    static Logger& instance(const Type type, std::ostream &stream);

private:
    explicit Logger(const Type type, std::ostream &stream);

    std::ostream &_stream;
};

} // namespace avocado::core

#define LOG_INFORMATION(message)\
    avocado::core::Logger::instance(avocado::core::Logger::Type::Console, std::cout).logInformation(message)

#define LOG_ERROR(message)\
    avocado::core::Logger::instance(avocado::core::Logger::Type::Console, std::cout).logError(message)

#endif // ifndef AVOCADO_CORE_LOGGER
