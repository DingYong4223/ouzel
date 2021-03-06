// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_NETWORK_NETWORK_HPP
#define OUZEL_NETWORK_NETWORK_HPP

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <winsock.h>
#endif

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ouzel
{
    namespace network
    {
        class Client;

        const uint32_t ANY_ADDRESS = 0;
        const uint16_t ANY_PORT = 0;

        class Network final
        {
        public:
            Network();
            ~Network();

            Network(const Network&) = delete;
            Network& operator=(const Network&) = delete;

            Network(Network&&) = delete;
            Network& operator=(Network&&) = delete;

            static uint32_t getAddress(const std::string& address);

            void listen(const std::string& address, uint16_t port);
            void connect(const std::string& address, uint16_t port);
            void disconnect();

        private:
#ifdef _WIN32
            bool wsaStarted = false;
            SOCKET endpoint = INVALID_SOCKET;
#else
            int endpoint = -1;
#endif

            std::vector<std::unique_ptr<Client>> clients;
        };
    } // namespace network
} // namespace ouzel

#endif // OUZEL_NETWORK_NETWORK_HPP
