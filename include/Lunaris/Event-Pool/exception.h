#pragma once

#include <stdexcept>

namespace Lunaris {
namespace EventPool {

    class EventPoolException : public std::runtime_error {
    public:
        explicit EventPoolException(const std::string&) noexcept;
        explicit EventPoolException(const char*) noexcept;

        const char* what() const noexcept;
    };

    class EventPoolTimeoutException : public EventPoolException {
    public:
        explicit EventPoolTimeoutException(const std::string&) noexcept;
        explicit EventPoolTimeoutException(const char*) noexcept;

        const char* what() const noexcept;
    };

} // namespace EventPool
} // namespace Lunaris