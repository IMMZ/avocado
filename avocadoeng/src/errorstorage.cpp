#include "errorstorage.hpp"

namespace avocado {

namespace core {

bool ErrorStorage::hasError() const noexcept {
    return _hasError;
}

void ErrorStorage::setHasError(bool he) const noexcept {
    _hasError = he;
}

const std::string& ErrorStorage::getErrorMessage() const noexcept {
    return _msg;
}

void ErrorStorage::setErrorMessage(const std::string &msg) const {
    _msg = msg;
}

void ErrorStorage::setErrorMessage(std::string &&msg) const noexcept {
    _msg = std::move(msg);
}

} // namespace core.

} // namespace avocado.

