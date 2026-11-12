#pragma once

#include <cstddef>
#include <cstdint>

#include "kv/Socket.hpp"

namespace kv {

class Server {
public:
    // Creates, binds and listens. Throws std::runtime_error on failure
    explicit Server(std::uint16_t port);

    // Not copyable
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Accept loop
    void run();

private:
    // Per-connection thread body
    static void handleClient(Socket client);

    // Sends all len bytes, looping on partial sends. Returns false if the connection is gone.
    static bool sendAll(int fd, const char* data, std::size_t len);

    Socket listener;
};

}  // namespace kv 