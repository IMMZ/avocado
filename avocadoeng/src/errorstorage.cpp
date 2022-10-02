#include "errorstorage.hpp"

namespace avocado {

namespace core {

bool ErrorStorage::hasError() const {
    return _hasError;
}

void ErrorStorage::setHasError(bool he) const {
    _hasError = he;
}

const std::string& ErrorStorage::getErrorMessage() const {
    return _msg;
}

void ErrorStorage::setErrorMessage(const std::string &msg) const {
    _msg = msg;
}

void ErrorStorage::setErrorMessage(std::string &&msg) const {
    _msg = std::move(msg);
}

} // namespace core.

} // namespace avocado.

