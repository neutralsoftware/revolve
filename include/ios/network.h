#pragma once

#include "ios/ios.h"
#include <functional>
#include <unordered_map>

class NetworkDevice final : public IOSDevice {
  public:
    explicit NetworkDevice(IOS &ios) : ios(ios) {}
    ~NetworkDevice() override;
    IOSResult ioctl(const IOSIoctlRequest &request) override;
    IOSResult ioctlv(const IOSIoctlvRequest &request,
                     const std::vector<IOSVector> &vectors) override;
    void update();

  private:
    struct Socket {
        int host = -1;
        bool nonblocking = false;
    };
    struct Pending {
        uint32_t address;
        std::function<IOSResult()> operation;
    };
    IOS &ios;
    std::unordered_map<int32_t, Socket> sockets;
    std::vector<Pending> pending;
    int32_t nextSocket = 0;
    uint64_t nextPoll = 0;
    int32_t addSocket(int host);
    IOSResult defer(uint32_t address, std::function<IOSResult()> operation);
};

void registerNetworkServices(IOS &ios);
