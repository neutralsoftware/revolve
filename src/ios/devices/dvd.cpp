#include "core/utils.h"
#include "ios/ios.h"
#include <algorithm>
#include <cstdint>
#include <span>

int32_t DIDevice::ioctl(const IOSIoctlRequest &request) {
    switch (static_cast<DIIoctl>(request.request)) {
    case DIIoctl::ReadDiskID: {
        if (!disc || !disc->isOpen())
            return static_cast<int32_t>(DIResult::DriveError);

        if (request.outSize < 0x20)
            return static_cast<int32_t>(DIResult::ReadTimedOut);

        std::array<uint8_t, 0x20> diskID{};

        if (!disc->readRaw(0, diskID))
            return static_cast<int32_t>(DIResult::DriveError);

        memory.writeBlock(request.outPtr, std::span<uint8_t>(diskID));

        discIDRead = true;
        lastLength = 0x20;

        Logger::log("DI", LogLevel::Info,
                    "DVDLowReadDiskID: " + disc->getGameID());

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::GetCoverStatus: {
        if (request.outSize < 4)
            return static_cast<int32_t>(DIResult::SecurityError);

        const uint32_t status = (disc && disc->isOpen()) ? 2 : 1;

        memory.write32(request.outPtr, status);

        Logger::log("DI", LogLevel::Info,
                    "DVDLowGetCoverStatus: " + std::to_string(status));

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::GetCoverRegister: {
        if (request.outSize < 4)
            return static_cast<int32_t>(DIResult::SecurityError);

        const uint32_t value = (disc && disc->isOpen()) ? 1 : 0;

        memory.write32(request.outPtr, value);

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::Reset: {
        currentPartition.reset();
        discIDRead = false;
        lastLength = 0;
        lastDriveError = 0;

        Logger::log("DI", LogLevel::Info, "DVDLowReset");

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::GetLength: {
        if (request.outSize < 4)
            return static_cast<int32_t>(DIResult::SecurityError);

        memory.write32(request.outPtr, lastLength);
        Logger::log("DI", LogLevel::Info,
                    "DVDLowGetLength: " + std::to_string(lastLength));

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::Read: {
        if (!disc || !disc->isOpen() || !currentPartition)
            return static_cast<int32_t>(DIResult::DriveError);
        if (request.inSize < 0x20)
            return static_cast<int32_t>(DIResult::BadArgument);

        uint32_t length = memory.read32(request.inPtr + 4);
        uint64_t offset =
            static_cast<uint64_t>(memory.read32(request.inPtr + 8)) << 2;
        if (request.outSize < length)
            return static_cast<int32_t>(DIResult::ReadTimedOut);
        if ((request.outPtr & 0x1F) != 0 || (length & 0x1F) != 0)
            return static_cast<int32_t>(DIResult::BadArgument);

        auto partitions = disc->getPartitions();
        auto partition =
            std::find_if(partitions.begin(), partitions.end(),
                         [&](const DiscPartition &candidate) {
                             return candidate.offset == *currentPartition;
                         });
        if (partition == partitions.end())
            return static_cast<int32_t>(DIResult::DriveError);

        std::vector<uint8_t> buffer(length);
        if (!disc->readPartition(*partition, offset, buffer)) {
            lastDriveError = 0x052100;
            return static_cast<int32_t>(DIResult::DriveError);
        }
        memory.writeBlock(request.outPtr, std::span<uint8_t>(buffer));
        lastLength = length;
        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::UnencryptedRead: {
        if (!disc || !disc->isOpen())
            return static_cast<int32_t>(DIResult::DriveError);

        if (request.inSize < 0x20)
            return static_cast<int32_t>(DIResult::BadArgument);

        const uint32_t length = memory.read32(request.inPtr + 4);

        const uint32_t position = memory.read32(request.inPtr + 8);

        const uint64_t offset = static_cast<uint64_t>(position) << 2;

        if (request.outSize < length)
            return static_cast<int32_t>(DIResult::ReadTimedOut);

        if ((request.outPtr & 0x1F) != 0 || (length & 0x1F) != 0) {
            return static_cast<int32_t>(DIResult::BadArgument);
        }

        std::vector<uint8_t> buffer(length);

        if (!disc->readRaw(offset, buffer)) {
            lastDriveError = 0x052100;

            return static_cast<int32_t>(DIResult::DriveError);
        }

        memory.writeBlock(request.outPtr, std::span<uint8_t>(buffer));

        lastLength = length;

        Logger::log("DI", LogLevel::Info,
                    "DVDLowUnencryptedRead offset=0x" +
                        utils::toHexString(offset) + " length=0x" +
                        utils::toHexString(length));

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::RequestError: {
        if (request.outSize >= 4)
            memory.write32(request.outPtr, lastDriveError);

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::ClosePartition: {
        currentPartition.reset();

        Logger::log("DI", LogLevel::Info, "DVDLowClosePartition");

        return static_cast<int32_t>(DIResult::Success);
    }
    case DIIoctl::WaitForCoverClose: {
        if (disc && disc->isOpen())
            return static_cast<int32_t>(DIResult::CoverClosed);

        return static_cast<int32_t>(DIResult::DriveError);
    }
    default:
        Logger::log("DI", LogLevel::Warning,
                    "Unknown DI ioctl: 0x" +
                        utils::toHexString(request.request));
        return static_cast<int32_t>(DIResult::DriveError);
    }

    return static_cast<int32_t>(IOSError::Invalid);
}

int32_t DIDevice::ioctlv(const IOSIoctlvRequest &request,
                         const std::vector<IOSVector> &vectors) {

    switch (static_cast<DIIoctl>(request.request)) {

    case DIIoctl::OpenPartition: {
        if (!disc || !disc->isOpen())
            return static_cast<int32_t>(DIResult::DriveError);

        if (!discIDRead)
            return static_cast<int32_t>(DIResult::BadArgument);

        if (request.inCount != 3 || request.outCount != 2 ||
            vectors.size() != 5) {

            return static_cast<int32_t>(DIResult::BadArgument);
        }

        if (vectors[0].size != 0x20)
            return static_cast<int32_t>(DIResult::BadArgument);

        const uint32_t partitionOffsetWords =
            memory.read32(vectors[0].address + 4);

        const uint64_t partitionOffset =
            static_cast<uint64_t>(partitionOffsetWords) << 2;

        Logger::log("DI", LogLevel::Info,
                    "DVDLowOpenPartition offset=0x" +
                        utils::toHexString(partitionOffset));

        const uint32_t tmdSize = disc->readBE32(partitionOffset + 0x2A4);

        const uint64_t tmdOffset =
            static_cast<uint64_t>(disc->readBE32(partitionOffset + 0x2A8)) << 2;

        if (tmdSize > vectors[3].size)
            return static_cast<int32_t>(DIResult::SecurityError);

        std::vector<uint8_t> tmd(tmdSize);

        if (!disc->readRaw(partitionOffset + tmdOffset, tmd)) {
            return static_cast<int32_t>(DIResult::DriveError);
        }

        memory.writeBlock(vectors[3].address, std::span<uint8_t>(tmd));

        if (vectors[4].size >= 4)
            memory.write32(vectors[4].address, 0);

        currentPartition = partitionOffset;

        return static_cast<int32_t>(DIResult::Success);
    }

    default:
        Logger::log("DI", LogLevel::Warning,
                    "Unhandled DI ioctlv=0x" +
                        utils::toHexString(request.request));

        return static_cast<int32_t>(DIResult::BadArgument);
    }
}
