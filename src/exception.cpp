#include <Lunaris/Event-Pool/exception.h>

namespace Lunaris {
namespace EventPool {

    EventPoolException::EventPoolException(const std::string& msg) noexcept
        : std::runtime_error(msg)
    {
    }

    EventPoolException::EventPoolException(const char* msg) noexcept
        : std::runtime_error(msg)
    {
    }

    const char* EventPoolException::what() const noexcept {
        return std::runtime_error::what();
    }

} // namespace Socket
} // namespace Lunaris