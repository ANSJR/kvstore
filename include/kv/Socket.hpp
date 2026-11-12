#pragma once

#include <unistd.h> // For close
#include <utility>

namespace kv {

class Socket {
public:
    Socket() = default;
    explicit Socket(int fd) : fd(fd) {}

    ~Socket() { reset(); }

    // No copies
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Move the fd, leave the source empty (-1) so destructor does nothing
    Socket(Socket&& other) noexcept : fd(std::exchange(other.fd, -1)) {}

    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            reset();
            fd = std::exchange(other.fd, -1);
        }
        return *this;
    }

    int get() const { return fd; }
    bool valid() const { return fd >= 0; }

    // Close
    void reset() {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

private:
    int fd = -1;
};

}  // namespace kv