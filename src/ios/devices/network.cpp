#include "ios/network.h"
#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <future>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
int32_t socketError(int error = errno) {
    switch (error) {
    case 0:
        return 0;
    case EACCES:
        return -2;
    case EADDRINUSE:
        return -3;
    case EADDRNOTAVAIL:
        return -4;
    case EAFNOSUPPORT:
        return -5;
    case EAGAIN:
        return -6;
    case EALREADY:
        return -7;
    case EBADF:
        return -8;
    case ECANCELED:
        return -11;
    case ECONNABORTED:
        return -13;
    case ECONNREFUSED:
        return -14;
    case ECONNRESET:
        return -15;
    case EDESTADDRREQ:
        return -17;
    case EHOSTUNREACH:
        return -23;
    case EINPROGRESS:
        return -26;
    case EINTR:
        return -27;
    case EINVAL:
        return -28;
    case EISCONN:
        return -30;
    case EMFILE:
        return -33;
    case EMSGSIZE:
        return -35;
    case ENETDOWN:
        return -38;
    case ENETRESET:
        return -39;
    case ENETUNREACH:
        return -40;
    case ENOBUFS:
        return -42;
    case ENOMEM:
        return -49;
    case ENOPROTOOPT:
        return -51;
    case ENOTCONN:
        return -56;
    case ENOTSOCK:
        return -59;
    case EOPNOTSUPP:
        return -63;
    case EPIPE:
        return -66;
    case EPROTONOSUPPORT:
        return -68;
    case ETIMEDOUT:
        return -76;
    default:
        return -29;
    }
}

bool wouldBlock() { return errno == EAGAIN || errno == EWOULDBLOCK; }

std::optional<sockaddr_in> readAddress(uint32_t pointer, uint32_t size) {
    if (size < 8 || Bus::readPhysical8(pointer) < 8 ||
        Bus::readPhysical8(pointer + 1) != 2)
        return std::nullopt;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(Bus::readPhysical16(pointer + 2));
    address.sin_addr.s_addr = htonl(Bus::readPhysical32(pointer + 4));
    return address;
}

void writeAddress(uint32_t pointer, const sockaddr_in &address) {
    Bus::writePhysical8(pointer, 8);
    Bus::writePhysical8(pointer + 1, 2);
    Bus::writePhysical16(pointer + 2, ntohs(address.sin_port));
    Bus::writePhysical32(pointer + 4, ntohl(address.sin_addr.s_addr));
}

uint32_t hostAddress() {
    ifaddrs *interfaces = nullptr;
    if (getifaddrs(&interfaces) != 0)
        return 0;
    uint32_t result = 0;
    for (auto *entry = interfaces; entry; entry = entry->ifa_next) {
        if (!entry->ifa_addr || entry->ifa_addr->sa_family != AF_INET)
            continue;
        const auto *address =
            reinterpret_cast<const sockaddr_in *>(entry->ifa_addr);
        const uint32_t candidate = ntohl(address->sin_addr.s_addr);
        if ((candidate >> 24) != 127 && candidate) {
            result = candidate;
            break;
        }
    }
    freeifaddrs(interfaces);
    return result;
}

int messageFlags(uint32_t flags) {
    int result = 0;
    if (flags & 1)
        result |= MSG_OOB;
    if (flags & 2)
        result |= MSG_PEEK;
#ifdef MSG_NOSIGNAL
    result |= MSG_NOSIGNAL;
#endif
    return result;
}
}

NetworkDevice::~NetworkDevice() {
    for (const auto &[id, socket] : sockets)
        ::close(socket.host);
}

int32_t NetworkDevice::addSocket(int host) {
    if (host < 0)
        return socketError();
    if (sockets.size() >= 24 || nextSocket == INT32_MAX) {
        ::close(host);
        return -33;
    }
    if (fcntl(host, F_SETFL, O_NONBLOCK) < 0) {
        const int32_t error = socketError();
        ::close(host);
        return error;
    }
#ifdef SO_NOSIGPIPE
    int enabled = 1;
    setsockopt(host, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled));
#endif
    const int32_t id = nextSocket++;
    sockets.emplace(id, Socket{host, false});
    return id;
}

IOSResult NetworkDevice::defer(uint32_t address,
                               std::function<IOSResult()> operation) {
    if (pending.size() >= 64)
        return IOS::error(IOSError::QueueFull);
    auto result = operation();
    if (result)
        return result;
    pending.push_back({address, std::move(operation)});
    return std::nullopt;
}

void NetworkDevice::update() {
    if (pending.empty())
        return;
    const auto now = SDL_GetTicks();
    if (now < nextPoll)
        return;
    nextPoll = now + 1;
    for (auto it = pending.begin(); it != pending.end();) {
        auto result = it->operation();
        if (!result) {
            ++it;
            continue;
        }
        const auto address = it->address;
        it = pending.erase(it);
        ios.completeRequest(address, *result);
    }
}

IOSResult NetworkDevice::ioctl(const IOSIoctlRequest &request) {
    const auto word = [&](uint32_t offset) {
        return Bus::readPhysical32(request.inPtr + offset);
    };
    if (request.request == 0x1F)
        return 0;
    if (request.request == 0x1B) {
        for (const auto &[id, socket] : sockets)
            ::close(socket.host);
        sockets.clear();
        return 0;
    }
    if (request.request == 0x10)
        return static_cast<int32_t>(hostAddress());
    if (request.request == 0x0F) {
        if (request.inSize < 12 || word(0) != 2 ||
            (word(4) != 1 && word(4) != 2))
            return -28;
        if (word(8) != 0 && word(8) != 6 && word(8) != 17)
            return -68;
        return addSocket(::socket(
            AF_INET, word(4) == 1 ? SOCK_STREAM : SOCK_DGRAM, word(8)));
    }
    if (request.request == 0x11) {
        if (!request.inSize || request.inSize > 256 || request.outSize < 0x460)
            return -28;
        std::string name;
        for (uint32_t i = 0; i < request.inSize; ++i) {
            const char c = Bus::readPhysical8(request.inPtr + i);
            if (!c)
                break;
            name += c;
        }
        if (name.empty() || name.size() == request.inSize ||
            pending.size() >= 64)
            return -28;
        auto future = std::make_shared<std::future<std::vector<uint32_t>>>(
            std::async(std::launch::async, [name] {
                addrinfo hints{};
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_STREAM;
                addrinfo *addresses = nullptr;
                std::vector<uint32_t> result;
                if (getaddrinfo(name.c_str(), nullptr, &hints, &addresses) ==
                    0) {
                    for (auto *entry = addresses; entry && result.size() < 16;
                         entry = entry->ai_next) {
                        auto *address =
                            reinterpret_cast<sockaddr_in *>(entry->ai_addr);
                        const uint32_t value = ntohl(address->sin_addr.s_addr);
                        if (std::find(result.begin(), result.end(), value) ==
                            result.end())
                            result.push_back(value);
                    }
                    freeaddrinfo(addresses);
                }
                return result;
            }));
        return defer(
            request.ipcAddress, [future, name, request]() -> IOSResult {
                if (future->wait_for(std::chrono::seconds(0)) !=
                    std::future_status::ready)
                    return std::nullopt;
                const auto addresses = future->get();
                if (addresses.empty())
                    return -23;
                const uint32_t base = request.outPtr;
                for (uint32_t i = 0; i < 0x460; ++i)
                    Bus::writePhysical8(base + i, 0);
                Bus::writePhysical32(base, base + 0x10);
                Bus::writePhysical32(base + 4, base + 0x110);
                Bus::writePhysical16(base + 8, 2);
                Bus::writePhysical16(base + 10, 4);
                Bus::writePhysical32(base + 12, base + 0x114);
                for (size_t i = 0; i < name.size(); ++i)
                    Bus::writePhysical8(base + 0x10 + i, name[i]);
                for (size_t i = 0; i < addresses.size(); ++i) {
                    Bus::writePhysical32(base + 0x114 + i * 4,
                                         base + 0x200 + i * 4);
                    Bus::writePhysical32(base + 0x200 + i * 4, addresses[i]);
                }
                return 0;
            });
    }
    if (request.request == 0x0B) {
        if (request.inSize < 8 || request.outSize % 12 ||
            request.outSize > 24 * 12)
            return -28;
        const int64_t timeout =
            static_cast<int64_t>((uint64_t(word(0)) << 32) | word(4));
        const uint64_t start = SDL_GetTicks();
        std::vector<std::pair<int32_t, uint32_t>> entries;
        for (uint32_t i = 0; i < request.outSize; i += 12)
            entries.emplace_back(
                static_cast<int32_t>(Bus::readPhysical32(request.outPtr + i)),
                Bus::readPhysical32(request.outPtr + i + 4));
        return defer(
            request.ipcAddress,
            [this, entries, start, timeout, request]() -> IOSResult {
                int ready = 0;
                for (size_t i = 0; i < entries.size(); ++i) {
                    const auto [id, events] = entries[i];
                    uint16_t returned = 0;
                    auto it = sockets.find(id);
                    if (id >= 0 && it == sockets.end()) {
                        returned = 0x20;
                    } else if (it != sockets.end()) {
                        pollfd descriptor{it->second.host, 0, 0};
                        if (events & 1)
                            descriptor.events |= POLLIN;
                        if (events & 2)
                            descriptor.events |= POLLPRI;
                        if (events & 4)
                            descriptor.events |= POLLOUT;
                        if (::poll(&descriptor, 1, 0) < 0)
                            return socketError();
                        if (descriptor.revents & POLLIN)
                            returned |= 1;
                        if (descriptor.revents & POLLPRI)
                            returned |= 2;
                        if (descriptor.revents & POLLOUT)
                            returned |= 4;
                        if (descriptor.revents & POLLERR)
                            returned |= 8;
                        if (descriptor.revents & POLLHUP)
                            returned |= 0x10;
                        if (descriptor.revents & POLLNVAL)
                            returned |= 0x20;
                    }
                    Bus::writePhysical32(request.outPtr + i * 12 + 8, returned);
                    ready += returned != 0;
                }
                if (ready || timeout == 0 ||
                    (timeout > 0 &&
                     SDL_GetTicks() - start >= static_cast<uint64_t>(timeout)))
                    return ready;
                return std::nullopt;
            });
    }
    if (request.inSize < 4)
        return -28;
    const int32_t id = word(0);
    auto it = sockets.find(id);
    if (it == sockets.end())
        return -8;
    const int host = it->second.host;
    const bool nonblocking = it->second.nonblocking;
    switch (request.request) {
    case 1:
        if (request.outSize && request.outSize < 8)
            return -28;
        return defer(request.ipcAddress,
                     [this, id, nonblocking, request]() -> IOSResult {
                         auto it = sockets.find(id);
                         if (it == sockets.end())
                             return -8;
                         sockaddr_in address{};
                         socklen_t length = sizeof(address);
                         const int accepted = ::accept(
                             it->second.host,
                             reinterpret_cast<sockaddr *>(&address), &length);
                         if (accepted < 0) {
                             if (wouldBlock() && !nonblocking)
                                 return std::nullopt;
                             return socketError();
                         }
                         const int32_t result = addSocket(accepted);
                         if (result >= 0 && request.outSize >= 8)
                             writeAddress(request.outPtr, address);
                         return result;
                     });
    case 2:
    case 4: {
        if (request.inSize < 16 || !word(4))
            return -28;
        auto address = readAddress(request.inPtr + 8, request.inSize - 8);
        if (!address)
            return -28;
        if (request.request == 2)
            return ::bind(host, reinterpret_cast<sockaddr *>(&*address),
                          sizeof(*address)) == 0
                       ? 0
                       : socketError();
        const int result = ::connect(
            host, reinterpret_cast<sockaddr *>(&*address), sizeof(*address));
        if (result == 0)
            return 0;
        if (errno != EINPROGRESS || nonblocking)
            return socketError();
        const uint64_t start = SDL_GetTicks();
        return defer(request.ipcAddress, [this, id, start]() -> IOSResult {
            auto it = sockets.find(id);
            if (it == sockets.end())
                return -8;
            pollfd descriptor{it->second.host, POLLOUT, 0};
            if (::poll(&descriptor, 1, 0) < 0)
                return socketError();
            if (!descriptor.revents)
                return SDL_GetTicks() - start > 30000 ? IOSResult(-76)
                                                      : std::nullopt;
            int error = 0;
            socklen_t size = sizeof(error);
            if (getsockopt(it->second.host, SOL_SOCKET, SO_ERROR, &error,
                           &size) < 0)
                return socketError();
            return socketError(error);
        });
    }
    case 3:
        ::close(host);
        sockets.erase(it);
        return 0;
    case 5:
        if (request.inSize < 12)
            return -28;
        if (word(4) == 3)
            return nonblocking ? 4 : 0;
        if (word(4) != 4)
            return -28;
        it->second.nonblocking = (word(8) & 4) != 0;
        return 0;
    case 6:
    case 7: {
        if (request.outSize < 8)
            return -28;
        sockaddr_in address{};
        socklen_t length = sizeof(address);
        const int result =
            request.request == 6
                ? getpeername(host, reinterpret_cast<sockaddr *>(&address),
                              &length)
                : getsockname(host, reinterpret_cast<sockaddr *>(&address),
                              &length);
        if (result < 0)
            return socketError();
        writeAddress(request.outPtr, address);
        return 0;
    }
    case 9: {
        if (request.inSize < 20 || word(12) != 4)
            return -28;
        int level = 0;
        int option = 0;
        if (word(4) == 0xFFFF) {
            level = SOL_SOCKET;
            switch (word(8)) {
            case 4:
                option = SO_REUSEADDR;
                break;
            case 8:
                option = SO_KEEPALIVE;
                break;
            case 0x20:
                option = SO_BROADCAST;
                break;
            case 0x1001:
                option = SO_SNDBUF;
                break;
            case 0x1002:
                option = SO_RCVBUF;
                break;
            default:
                return -51;
            }
        } else if (word(4) == 6 && word(8) == 1) {
            level = IPPROTO_TCP;
            option = TCP_NODELAY;
        } else
            return -51;
        int value = word(16);
        return setsockopt(host, level, option, &value, sizeof(value)) == 0
                   ? 0
                   : socketError();
    }
    case 10:
        if (request.inSize < 8)
            return -28;
        return ::listen(host, std::min(word(4), 128u)) == 0 ? 0 : socketError();
    case 14:
        if (request.inSize < 8 || word(4) > 2)
            return -28;
        return ::shutdown(host, word(4)) == 0 ? 0 : socketError();
    default:
        return -63;
    }
}

IOSResult NetworkDevice::ioctlv(const IOSIoctlvRequest &request,
                                const std::vector<IOSVector> &vectors) {
    if (request.request == 0x1C) {
        if (request.inCount != 1 || request.outCount != 2 ||
            vectors.size() != 3 || vectors[0].size < 8 || vectors[2].size < 4)
            return -28;
        const uint32_t option = Bus::readPhysical32(vectors[0].address + 4);
        uint32_t size = 0;
        switch (option) {
        case 0x1004:
            size = 6;
            break;
        case 0x1005:
            size = 4;
            break;
        case 0x4003:
            size = 12;
            break;
        case 0x4005:
            size = 4;
            break;
        case 0x4006:
            size = 0;
            break;
        default:
            return -51;
        }
        if (vectors[1].size < size)
            return -28;
        for (uint32_t i = 0; i < size; ++i)
            Bus::writePhysical8(vectors[1].address + i, 0);
        if (option == 0x1004) {
            constexpr std::array<uint8_t, 6> mac{0x02, 0x52, 0x56,
                                                 0x4C, 0x56, 0x01};
            for (size_t i = 0; i < mac.size(); ++i)
                Bus::writePhysical8(vectors[1].address + i, mac[i]);
        } else if (option == 0x1005) {
            Bus::writePhysical32(vectors[1].address, hostAddress() ? 1 : 0);
        } else if (option == 0x4003) {
            const auto ip = hostAddress();
            Bus::writePhysical32(vectors[1].address, ip);
            ifaddrs *interfaces = nullptr;
            if (getifaddrs(&interfaces) == 0) {
                for (auto *entry = interfaces; entry; entry = entry->ifa_next) {
                    if (!entry->ifa_addr ||
                        entry->ifa_addr->sa_family != AF_INET ||
                        !entry->ifa_netmask)
                        continue;
                    const auto *address =
                        reinterpret_cast<const sockaddr_in *>(entry->ifa_addr);
                    if (ntohl(address->sin_addr.s_addr) != ip)
                        continue;
                    const auto mask =
                        ntohl(reinterpret_cast<const sockaddr_in *>(
                                  entry->ifa_netmask)
                                  ->sin_addr.s_addr);
                    Bus::writePhysical32(vectors[1].address + 4, mask);
                    Bus::writePhysical32(vectors[1].address + 8, ip | ~mask);
                    break;
                }
                freeifaddrs(interfaces);
            }
        }
        Bus::writePhysical32(vectors[2].address, size);
        return 0;
    }
    if (request.request != 12 && request.request != 13)
        return -63;
    const bool sending = request.request == 13;
    if ((sending &&
         (request.inCount != 2 || request.outCount || vectors.size() != 2)) ||
        (!sending && (request.inCount != 1 || request.outCount != 2 ||
                      vectors.size() != 3)))
        return -28;
    const auto parameters = vectors[sending ? 1 : 0];
    const auto data = vectors[sending ? 0 : 1];
    if (parameters.size < (sending ? 12u : 8u) || data.size > 1024 * 1024)
        return -28;
    const int32_t id = Bus::readPhysical32(parameters.address);
    const uint32_t flags = Bus::readPhysical32(parameters.address + 4);
    if (flags & ~0x47u)
        return -28;
    auto socket = sockets.find(id);
    if (socket == sockets.end())
        return -8;
    const bool nonblocking = socket->second.nonblocking || (flags & 0x44);
    std::optional<sockaddr_in> destination;
    if (sending && Bus::readPhysical32(parameters.address + 8)) {
        destination =
            readAddress(parameters.address + 12, parameters.size - 12);
        if (!destination)
            return -28;
    }
    const IOSVector source = sending ? IOSVector{} : vectors[2];
    if (source.size && source.size < 8)
        return -28;
    auto bytes = std::make_shared<std::vector<uint8_t>>(data.size);
    if (sending) {
        for (uint32_t i = 0; i < data.size; ++i)
            (*bytes)[i] = Bus::readPhysical8(data.address + i);
    }
    return defer(
        request.ipcAddress,
        [this, id, flags, nonblocking, sending, destination, data, source,
         bytes]() -> IOSResult {
            auto socket = sockets.find(id);
            if (socket == sockets.end())
                return -8;
            sockaddr_in peer{};
            socklen_t peerLength = sizeof(peer);
            const ssize_t result =
                sending
                    ? ::sendto(socket->second.host, bytes->data(),
                               bytes->size(), messageFlags(flags),
                               destination ? reinterpret_cast<const sockaddr *>(
                                                 &*destination)
                                           : nullptr,
                               destination ? sizeof(*destination) : 0)
                    : ::recvfrom(socket->second.host, bytes->data(),
                                 bytes->size(), messageFlags(flags),
                                 reinterpret_cast<sockaddr *>(&peer),
                                 &peerLength);
            if (result < 0) {
                if (wouldBlock() && !nonblocking)
                    return std::nullopt;
                return socketError();
            }
            if (!sending) {
                for (uint32_t i = 0; i < static_cast<uint32_t>(result); ++i)
                    Bus::writePhysical8(data.address + i, (*bytes)[i]);
                if (source.size >= 8)
                    writeAddress(source.address, peer);
            }
            return static_cast<int32_t>(result);
        });
}
