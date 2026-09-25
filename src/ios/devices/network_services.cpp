#include "ios/network.h"
#include <array>
#include <chrono>
#include <fstream>

namespace {
void clearOutput(uint32_t address, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i)
        Bus::writePhysical8(address + i, 0);
}

class NetworkConfiguration final : public IOSDevice {
  public:
    NetworkConfiguration() {
        const auto path =
            FSDevice::rootPath() / "shared2/sys/net/02/config.dat";
        std::ifstream input(path, std::ios::binary);
        if (input) {
            input.read(reinterpret_cast<char *>(configuration.data()),
                       configuration.size());
            if (input.gcount() ==
                static_cast<std::streamsize>(configuration.size()))
                return;
        }
        configuration.fill(0);
        configuration[4] = 1;
        configuration[6] = 7;
        configuration[8] = 0xA7;
    }

    IOSResult ioctlv(const IOSIoctlvRequest &request,
                     const std::vector<IOSVector> &vectors) override {
        if (request.inCount > vectors.size() ||
            request.outCount != vectors.size() - request.inCount ||
            !request.outCount)
            return IOS::error(IOSError::Invalid);
        const auto result = vectors.back();
        if (request.request == 8) {
            if (request.inCount || request.outCount != 2 ||
                vectors[0].size < 4 || vectors[1].size < 6)
                return IOS::error(IOSError::Invalid);
            clearOutput(vectors[0].address, vectors[0].size);
            constexpr std::array<uint8_t, 6> mac{0x02, 0x52, 0x56,
                                                 0x4C, 0x56, 0x01};
            for (size_t i = 0; i < mac.size(); ++i)
                Bus::writePhysical8(vectors[1].address + i, mac[i]);
            return 0;
        }
        if (result.size < 8 || result.size > 32)
            return IOS::error(IOSError::Invalid);
        switch (request.request) {
        case 1:
        case 2:
            if ((request.request == 1 && request.inCount) ||
                (request.request == 2 &&
                 (request.inCount != 1 || vectors[0].size < 4)))
                return IOS::error(IOSError::Invalid);
            break;
        case 3:
        case 5:
            if (request.inCount || request.outCount != 2 ||
                vectors[0].size < configuration.size())
                return IOS::error(IOSError::Invalid);
            for (size_t i = 0; i < configuration.size(); ++i)
                Bus::writePhysical8(vectors[0].address + i, configuration[i]);
            break;
        case 4:
        case 6: {
            if (request.inCount != 1 || request.outCount != 1 ||
                vectors[0].size != configuration.size())
                return IOS::error(IOSError::Invalid);
            std::array<uint8_t, 7004> replacement{};
            for (size_t i = 0; i < replacement.size(); ++i)
                replacement[i] = Bus::readPhysical8(vectors[0].address + i);
            if (request.request == 6) {
                const auto path =
                    FSDevice::rootPath() / "shared2/sys/net/02/config.dat";
                std::error_code ec;
                std::filesystem::create_directories(path.parent_path(), ec);
                if (ec)
                    return IOS::error(IOSError::FS_IO);
                std::ofstream output(path, std::ios::binary);
                output.write(reinterpret_cast<const char *>(replacement.data()),
                             replacement.size());
                if (!output)
                    return IOS::error(IOSError::FS_IO);
            }
            configuration = replacement;
            break;
        }
        case 7:
            if (request.inCount || request.outCount != 1)
                return IOS::error(IOSError::Invalid);
            break;
        default:
            return IOS::error(IOSError::Invalid);
        }
        clearOutput(result.address, result.size);
        if (request.request == 7)
            Bus::writePhysical32(result.address + 4, 1);
        return 0;
    }

  private:
    std::array<uint8_t, 7004> configuration{};
};

class WiiConnect24 final : public IOSDevice {
  public:
    IOSResult ioctl(const IOSIoctlRequest &request) override {
        if (request.outSize < 4 || request.outSize > 256)
            return IOS::error(IOSError::Invalid);
        clearOutput(request.outPtr, request.outSize);
        switch (request.request) {
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
        case 8:
        case 9:
        case 0x28:
            return 0;
        case 4:
            if (request.outSize < 12)
                return IOS::error(IOSError::Invalid);
            Bus::writePhysical32(request.outPtr + 4, downloadTrigger);
            Bus::writePhysical32(request.outPtr + 8, mailTrigger);
            return 0;
        case 5:
            if (request.inSize < 8)
                return IOS::error(IOSError::Invalid);
            if (Bus::readPhysical32(request.inPtr) >= 259200 ||
                Bus::readPhysical32(request.inPtr + 4) >= 259200)
                return IOS::error(IOSError::Invalid);
            downloadTrigger = Bus::readPhysical32(request.inPtr);
            mailTrigger = Bus::readPhysical32(request.inPtr + 4);
            return 0;
        case 0x1E:
            if (request.outSize < 16)
                return IOS::error(IOSError::Invalid);
            return 0;
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
            Bus::writePhysical32(request.outPtr, static_cast<uint32_t>(-9));
            return 0;
        default:
            return IOS::error(IOSError::Invalid);
        }
    }

  private:
    uint32_t downloadTrigger = 0;
    uint32_t mailTrigger = 0;
};

class NetworkClock final : public IOSDevice {
  public:
    IOSResult ioctl(const IOSIoctlRequest &request) override {
        if (request.request == 0x16)
            return -9;
        if (request.outSize < 4 || (request.outPtr & 3))
            return IOS::error(IOSError::Invalid);
        const int64_t now =
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
        switch (request.request) {
        case 0x14:
        case 0x18: {
            if (request.outSize < 12)
                return IOS::error(IOSError::Invalid);
            const uint64_t value =
                request.request == 0x14 ? now + offset : offset;
            Bus::writePhysical32(request.outPtr, 0);
            Bus::writePhysical32(request.outPtr + 4, value >> 32);
            Bus::writePhysical32(request.outPtr + 8, value);
            return 0;
        }
        case 0x15: {
            if (request.inSize < 12 || (request.inPtr & 3))
                return IOS::error(IOSError::Invalid);
            const uint64_t utc =
                (uint64_t(Bus::readPhysical32(request.inPtr)) << 32) |
                Bus::readPhysical32(request.inPtr + 4);
            if (utc < 0x30DF3B00 || utc > 0xDFAEF080) {
                Bus::writePhysical32(request.outPtr, static_cast<uint32_t>(-3));
                return 0;
            }
            offset = static_cast<int64_t>(utc) - now;
            Bus::writePhysical32(request.outPtr, 0);
            return 0;
        }
        default:
            return IOS::error(IOSError::Invalid);
        }
    }

  private:
    int64_t offset = 0;
};
}

void registerNetworkServices(IOS &ios) {
    ios.registerDevice("/dev/net/ncd/manage",
                       std::make_shared<NetworkConfiguration>());
    ios.registerDevice("/dev/net/kd/request", std::make_shared<WiiConnect24>());
    ios.registerDevice("/dev/net/kd/time", std::make_shared<NetworkClock>());
}
