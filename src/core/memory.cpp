#include "memory.h"
#include "core/memory.h"
#include "core/utils.h"
#include "device.h"
#include <cstdint>
#include <span>
#include <sys/types.h>
#include <vector>

ResolvedAddress Bus::resolveAddress(uint32_t addr) {
    if (addr < 0x01800000)
        return {MemoryRegion::MEM1, addr};
    if (addr >= MEM2_PHYS_START && addr < MEM2_PHYS_START + 0x04000000)
        return {MemoryRegion::MEM2, addr - MEM2_PHYS_START};

    if (addr >= MEM1_CACHED_START && addr <= MEM1_CACHED_END) {
        return {MemoryRegion::MEM1, addr - MEM1_CACHED_START};
    }

    if (addr >= MEM1_UNCACHED_START && addr <= MEM1_UNCACHED_END) {
        return {MemoryRegion::MEM1, addr - MEM1_UNCACHED_START};
    }

    if (addr >= MEM2_CACHED_START && addr <= MEM2_CACHED_END) {
        return {MemoryRegion::MEM2, addr - MEM2_CACHED_START};
    }

    if (addr >= MEM2_UNCACHED_START && addr <= MEM2_UNCACHED_END) {
        return {MemoryRegion::MEM2, addr - MEM2_UNCACHED_START};
    }

    if (addr >= MMIO_GAMECUBE_START && addr <= MMIO_GAMECUBE_END) {
        return {MemoryRegion::MMIO, addr};
    }

    if (addr >= MMIO_WII_START && addr <= MMIO_WII_END) {
        return {MemoryRegion::MMIO, addr};
    }

    return {MemoryRegion::Invalid, 0};
}

uint8_t Bus::read8(uint32_t addr) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        return mem.read8(resolved.offset, 1);
    case MemoryRegion::MEM2:
        return mem.read8(resolved.offset, 2);
    case MemoryRegion::MMIO:
        return Device::globalDevice->mmioDispatcher.read8(resolved.offset);
    default:
        Logger::log("Memory", LogLevel::Error,
                    "read8: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
        return 0;
    }
}

uint16_t Bus::read16(uint32_t addr) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        return mem.read16(resolved.offset, 1);
    case MemoryRegion::MEM2:
        return mem.read16(resolved.offset, 2);
    case MemoryRegion::MMIO:
        return Device::globalDevice->mmioDispatcher.read16(resolved.offset);
    default:
        Logger::log("Memory", LogLevel::Error,
                    "read16: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
        return 0;
    }
}

uint32_t Bus::read32(uint32_t addr) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        return mem.read32(resolved.offset, 1);
    case MemoryRegion::MEM2:
        return mem.read32(resolved.offset, 2);
    case MemoryRegion::MMIO:
        return Device::globalDevice->mmioDispatcher.read32(resolved.offset);
    default:
        Logger::log("Memory", LogLevel::Error,
                    "read32: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
        return 0;
    }
}

uint64_t Bus::read64(uint32_t addr) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        return mem.read64(resolved.offset, 1);
    case MemoryRegion::MEM2:
        return mem.read64(resolved.offset, 2);
    default:
        Logger::log("Memory", LogLevel::Error,
                    "read64: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
        return 0;
    }
}

float Bus::readFloat(uint32_t addr) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        return mem.readFloat(resolved.offset, 1);
    case MemoryRegion::MEM2:
        return mem.readFloat(resolved.offset, 2);
    default:
        Logger::log("Memory", LogLevel::Error,
                    "readFloat: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
        return 0.0f;
    }
}

double Bus::readDouble(uint32_t addr) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        return mem.readDouble(resolved.offset, 1);
    case MemoryRegion::MEM2:
        return mem.readDouble(resolved.offset, 2);
    default:
        Logger::log("Memory", LogLevel::Error,
                    "readDouble: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
        return 0.0;
    }
}

void Bus::write8(uint32_t addr, uint8_t value) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    auto &cpu = Device::globalDevice->cpu;
    if (cpu.state.reservationValid) {
        ResolvedAddress reserved = resolveAddress(cpu.state.reservationAddress);
        if (reserved.region == resolved.region &&
            (reserved.offset & ~31u) >= (resolved.offset & ~31u) &&
            (reserved.offset & ~31u) <= (resolved.offset & ~31u))
            cpu.state.reservationValid = false;
    }
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        mem.write8(resolved.offset, value, 1);
        break;
    case MemoryRegion::MEM2:
        mem.write8(resolved.offset, value, 2);
        break;
    case MemoryRegion::MMIO:
        Device::globalDevice->mmioDispatcher.write8(resolved.offset, value);
        break;
    default:
        Logger::log("Memory", LogLevel::Error,
                    "write8: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
    }
}

void Bus::write16(uint32_t addr, uint16_t value) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    auto &cpu = Device::globalDevice->cpu;
    if (cpu.state.reservationValid) {
        ResolvedAddress reserved = resolveAddress(cpu.state.reservationAddress);
        if (reserved.region == resolved.region &&
            (reserved.offset & ~31u) >= (resolved.offset & ~31u) &&
            (reserved.offset & ~31u) <= ((resolved.offset + 1) & ~31u))
            cpu.state.reservationValid = false;
    }
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        mem.write16(resolved.offset, value, 1);
        break;
    case MemoryRegion::MEM2:
        mem.write16(resolved.offset, value, 2);
        break;
    case MemoryRegion::MMIO:
        Device::globalDevice->mmioDispatcher.write16(resolved.offset, value);
        break;
    default:
        Logger::log("Memory", LogLevel::Error,
                    "write16: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
    }
}

void Bus::write32(uint32_t addr, uint32_t value) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    auto &cpu = Device::globalDevice->cpu;
    if (cpu.state.reservationValid) {
        ResolvedAddress reserved = resolveAddress(cpu.state.reservationAddress);
        if (reserved.region == resolved.region &&
            (reserved.offset & ~31u) >= (resolved.offset & ~31u) &&
            (reserved.offset & ~31u) <= ((resolved.offset + 3) & ~31u))
            cpu.state.reservationValid = false;
    }
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        mem.write32(resolved.offset, value, 1);
        break;
    case MemoryRegion::MEM2:
        mem.write32(resolved.offset, value, 2);
        break;
    case MemoryRegion::MMIO:
        Device::globalDevice->mmioDispatcher.write32(resolved.offset, value);
        break;
    default:
        Logger::log("Memory", LogLevel::Error,
                    "write32: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
    }
}

void Bus::write64(uint32_t addr, uint64_t value) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    auto &cpu = Device::globalDevice->cpu;
    if (cpu.state.reservationValid) {
        ResolvedAddress reserved = resolveAddress(cpu.state.reservationAddress);
        if (reserved.region == resolved.region &&
            (reserved.offset & ~31u) >= (resolved.offset & ~31u) &&
            (reserved.offset & ~31u) <= ((resolved.offset + 7) & ~31u))
            cpu.state.reservationValid = false;
    }
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        mem.write64(resolved.offset, value, 1);
        break;
    case MemoryRegion::MEM2:
        mem.write64(resolved.offset, value, 2);
        break;
    default:
        Logger::log("Memory", LogLevel::Error,
                    "write64: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
    }
}

void Bus::writeFloat(uint32_t addr, float value) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    auto &cpu = Device::globalDevice->cpu;
    if (cpu.state.reservationValid) {
        ResolvedAddress reserved = resolveAddress(cpu.state.reservationAddress);
        if (reserved.region == resolved.region &&
            (reserved.offset & ~31u) >= (resolved.offset & ~31u) &&
            (reserved.offset & ~31u) <= ((resolved.offset + 3) & ~31u))
            cpu.state.reservationValid = false;
    }
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        mem.writeFloat(resolved.offset, value, 1);
        break;
    case MemoryRegion::MEM2:
        mem.writeFloat(resolved.offset, value, 2);
        break;
    default:
        Logger::log("Memory", LogLevel::Error,
                    "writeFloat: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
    }
}

void Bus::writeDouble(uint32_t addr, double value) {
    ResolvedAddress resolved = resolveAddress(addr);
    Memory &mem = Device::globalDevice->memory;
    auto &cpu = Device::globalDevice->cpu;
    if (cpu.state.reservationValid) {
        ResolvedAddress reserved = resolveAddress(cpu.state.reservationAddress);
        if (reserved.region == resolved.region &&
            (reserved.offset & ~31u) >= (resolved.offset & ~31u) &&
            (reserved.offset & ~31u) <= ((resolved.offset + 7) & ~31u))
            cpu.state.reservationValid = false;
    }
    switch (resolved.region) {
    case MemoryRegion::MEM1:
        mem.writeDouble(resolved.offset, value, 1);
        break;
    case MemoryRegion::MEM2:
        mem.writeDouble(resolved.offset, value, 2);
        break;
    default:
        Logger::log("Memory", LogLevel::Error,
                    "writeDouble: Invalid memory region for address (" +
                        utils::toHexString(addr) + ")");
    }
}

void Memory::write8(uint32_t addr, uint8_t value, int memIndex) {
    if (memIndex == 1) {
        mem1[addr] = value;
    } else if (memIndex == 2) {
        mem2[addr] = value;
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "write8: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
    }
}

void Memory::write16(uint32_t addr, uint16_t value, int memIndex) {
    if (memIndex == 1) {
        mem1[addr] = static_cast<uint8_t>((value >> 8) & 0xFF);
        mem1[addr + 1] = static_cast<uint8_t>(value & 0xFF);
    } else if (memIndex == 2) {
        mem2[addr] = static_cast<uint8_t>((value >> 8) & 0xFF);
        mem2[addr + 1] = static_cast<uint8_t>(value & 0xFF);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "write16: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
    }
}

void Memory::write32(uint32_t addr, uint32_t value, int memIndex) {
    if (memIndex == 1) {
        mem1[addr] = static_cast<uint8_t>((value >> 24) & 0xFF);
        mem1[addr + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        mem1[addr + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        mem1[addr + 3] = static_cast<uint8_t>(value & 0xFF);
    } else if (memIndex == 2) {
        mem2[addr] = static_cast<uint8_t>((value >> 24) & 0xFF);
        mem2[addr + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        mem2[addr + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        mem2[addr + 3] = static_cast<uint8_t>(value & 0xFF);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "write32: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
    }
}

void Memory::write64(uint32_t addr, uint64_t value, int memIndex) {
    if (memIndex == 1) {
        mem1[addr] = static_cast<uint8_t>((value >> 56) & 0xFF);
        mem1[addr + 1] = static_cast<uint8_t>((value >> 48) & 0xFF);
        mem1[addr + 2] = static_cast<uint8_t>((value >> 40) & 0xFF);
        mem1[addr + 3] = static_cast<uint8_t>((value >> 32) & 0xFF);
        mem1[addr + 4] = static_cast<uint8_t>((value >> 24) & 0xFF);
        mem1[addr + 5] = static_cast<uint8_t>((value >> 16) & 0xFF);
        mem1[addr + 6] = static_cast<uint8_t>((value >> 8) & 0xFF);
        mem1[addr + 7] = static_cast<uint8_t>(value & 0xFF);
    } else if (memIndex == 2) {
        mem2[addr] = static_cast<uint8_t>((value >> 56) & 0xFF);
        mem2[addr + 1] = static_cast<uint8_t>((value >> 48) & 0xFF);
        mem2[addr + 2] = static_cast<uint8_t>((value >> 40) & 0xFF);
        mem2[addr + 3] = static_cast<uint8_t>((value >> 32) & 0xFF);
        mem2[addr + 4] = static_cast<uint8_t>((value >> 24) & 0xFF);
        mem2[addr + 5] = static_cast<uint8_t>((value >> 16) & 0xFF);
        mem2[addr + 6] = static_cast<uint8_t>((value >> 8) & 0xFF);
        mem2[addr + 7] = static_cast<uint8_t>(value & 0xFF);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "write64: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
    }
}

uint8_t Memory::read8(uint32_t addr, int memIndex) {
    if (memIndex == 1) {
        return mem1[addr];
    } else if (memIndex == 2) {
        return mem2[addr];
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "read8: Invalid memory index (" + std::to_string(memIndex) +
                        ")");
        return 0;
    }
}

uint16_t Memory::read16(uint32_t addr, int memIndex) {
    if (memIndex == 1) {
        return (static_cast<uint16_t>(mem1[addr]) << 8) |
               static_cast<uint16_t>(mem1[addr + 1]);
    } else if (memIndex == 2) {
        return (static_cast<uint16_t>(mem2[addr]) << 8) |
               static_cast<uint16_t>(mem2[addr + 1]);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "read16: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
        return 0;
    }
}

uint32_t Memory::read32(uint32_t addr, int memIndex) {
    if (memIndex == 1) {
        return (static_cast<uint32_t>(mem1[addr]) << 24) |
               (static_cast<uint32_t>(mem1[addr + 1]) << 16) |
               (static_cast<uint32_t>(mem1[addr + 2]) << 8) |
               static_cast<uint32_t>(mem1[addr + 3]);
    } else if (memIndex == 2) {
        return (static_cast<uint32_t>(mem2[addr]) << 24) |
               (static_cast<uint32_t>(mem2[addr + 1]) << 16) |
               (static_cast<uint32_t>(mem2[addr + 2]) << 8) |
               static_cast<uint32_t>(mem2[addr + 3]);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "read32: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
        return 0;
    }
}

uint64_t Memory::read64(uint32_t addr, int memIndex) {
    if (memIndex == 1) {
        return (static_cast<uint64_t>(mem1[addr]) << 56) |
               (static_cast<uint64_t>(mem1[addr + 1]) << 48) |
               (static_cast<uint64_t>(mem1[addr + 2]) << 40) |
               (static_cast<uint64_t>(mem1[addr + 3]) << 32) |
               (static_cast<uint64_t>(mem1[addr + 4]) << 24) |
               (static_cast<uint64_t>(mem1[addr + 5]) << 16) |
               (static_cast<uint64_t>(mem1[addr + 6]) << 8) |
               static_cast<uint64_t>(mem1[addr + 7]);
    } else if (memIndex == 2) {
        return (static_cast<uint64_t>(mem2[addr]) << 56) |
               (static_cast<uint64_t>(mem2[addr + 1]) << 48) |
               (static_cast<uint64_t>(mem2[addr + 2]) << 40) |
               (static_cast<uint64_t>(mem2[addr + 3]) << 32) |
               (static_cast<uint64_t>(mem2[addr + 4]) << 24) |
               (static_cast<uint64_t>(mem2[addr + 5]) << 16) |
               (static_cast<uint64_t>(mem2[addr + 6]) << 8) |
               static_cast<uint64_t>(mem2[addr + 7]);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "read64: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
        return 0;
    }
}

float Memory::readFloat(uint32_t addr, int memIndex) {
    uint32_t intValue = read32(addr, memIndex);
    float floatValue;
    std::memcpy(&floatValue, &intValue, sizeof(float));
    return floatValue;
}

double Memory::readDouble(uint32_t addr, int memIndex) {
    uint64_t intValue = read64(addr, memIndex);
    double doubleValue;
    std::memcpy(&doubleValue, &intValue, sizeof(double));
    return doubleValue;
}

void Memory::writeFloat(uint32_t addr, float value, int memIndex) {
    uint32_t intValue;
    std::memcpy(&intValue, &value, sizeof(float));
    write32(addr, intValue, memIndex);
}

void Memory::writeDouble(uint32_t addr, double value, int memIndex) {
    uint64_t intValue;
    std::memcpy(&intValue, &value, sizeof(double));
    write64(addr, intValue, memIndex);
}

void MMIO::registerDevice(uint32_t baseAddr, uint32_t size,
                          MMIODevice *device) {
    devices.push_back({baseAddr, size, device});
}

void MMIO::write8(uint32_t addr, uint8_t value) {
    for (const auto &entry : devices) {
        if (addr >= entry.baseAddr && addr < entry.baseAddr + entry.size) {
            entry.device->write(addr - entry.baseAddr, value, AccessSize::U8);
            return;
        }
    }
    Logger::log("Memory", LogLevel::Error,
                "MMIO write8: Address not mapped to any device (" +
                    utils::toHexString(addr) + ")");
    return;
}

void MMIO::write16(uint32_t addr, uint16_t value) {
    for (const auto &entry : devices) {
        if (addr >= entry.baseAddr && addr < entry.baseAddr + entry.size) {
            entry.device->write(addr - entry.baseAddr, value, AccessSize::U16);
            return;
        }
    }
    Logger::log("Memory", LogLevel::Error,
                "MMIO write16: Address not mapped to any device (" +
                    utils::toHexString(addr) + ")");
}

void MMIO::write32(uint32_t addr, uint32_t value) {
    for (const auto &entry : devices) {
        if (addr >= entry.baseAddr && addr < entry.baseAddr + entry.size) {
            entry.device->write(addr - entry.baseAddr, value, AccessSize::U32);
            return;
        }
    }
    Logger::log("Memory", LogLevel::Error,
                "MMIO write32: Address not mapped to any device (" +
                    utils::toHexString(addr) + ")");
}

uint8_t MMIO::read8(uint32_t addr) {
    for (const auto &entry : devices) {
        if (addr >= entry.baseAddr && addr < entry.baseAddr + entry.size) {
            return entry.device->read(addr - entry.baseAddr, AccessSize::U8);
        }
    }
    Logger::log("Memory", LogLevel::Error,
                "MMIO read8: Address not mapped to any device (" +
                    utils::toHexString(addr) + ")");
    return 0;
}

uint16_t MMIO::read16(uint32_t addr) {
    for (const auto &entry : devices) {
        if (addr >= entry.baseAddr && addr < entry.baseAddr + entry.size) {
            return entry.device->read(addr - entry.baseAddr, AccessSize::U16);
        }
    }
    Logger::log("Memory", LogLevel::Error,
                "MMIO read16: Address not mapped to any device (" +
                    utils::toHexString(addr) + ")");
    return 0;
}

uint32_t MMIO::read32(uint32_t addr) {
    for (const auto &entry : devices) {
        if (addr >= entry.baseAddr && addr < entry.baseAddr + entry.size) {
            return entry.device->read(addr - entry.baseAddr, AccessSize::U32);
        }
    }
    Logger::log("Memory", LogLevel::Error,
                "MMIO read32: Address not mapped to any device (" +
                    utils::toHexString(addr) + ")");
    return 0;
}

void Bus::writeFromStream(uint32_t addr, size_t count,
                          BigEndianStream &stream) {
    auto bytes = stream.readNBytes(count);
    writeBlock(addr, bytes);
}

void Bus::writeBlock(uint32_t addr, std::span<const uint8_t> bytes) {
    if (bytes.empty())
        return;

    auto first = resolveAddress(addr);
    auto last = resolveAddress(addr + static_cast<uint32_t>(bytes.size() - 1));

    if (first.region != last.region)
        throw std::runtime_error("Memory block crosses regions");

    Memory &memory = Device::globalDevice->memory;

    switch (first.region) {
    case MemoryRegion::MEM1:
        memory.writeBlock(first.offset, bytes, 1);
        break;

    case MemoryRegion::MEM2:
        memory.writeBlock(first.offset, bytes, 2);
        break;

    default:
        throw std::runtime_error("Cannot block-write this region");
    }
}

void Memory::writeBlock(uint32_t addr, std::span<const uint8_t> bytes,
                        int memIndex) {
    if (memIndex == 1) {
        std::copy(bytes.begin(), bytes.end(), mem1.begin() + addr);
    } else if (memIndex == 2) {
        std::copy(bytes.begin(), bytes.end(), mem2.begin() + addr);
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "writeBlock: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
    }
}

void Bus::readBlock(uint32_t addr, std::vector<uint8_t> &buffer) {
    if (buffer.empty())
        return;

    auto first = resolveAddress(addr);
    auto last = resolveAddress(addr + static_cast<uint32_t>(buffer.size() - 1));

    if (first.region != last.region)
        throw std::runtime_error("Memory block crosses regions");

    Memory &memory = Device::globalDevice->memory;

    switch (first.region) {
    case MemoryRegion::MEM1:
        memory.readBlock(first.offset, buffer, 1);
        break;

    case MemoryRegion::MEM2:
        memory.readBlock(first.offset, buffer, 2);
        break;

    default:
        throw std::runtime_error("Cannot block-read this region");
    }
}

void Memory::readBlock(uint32_t addr, std::vector<uint8_t> &buffer,
                       int memIndex) {
    if (memIndex == 1) {
        std::copy(mem1.begin() + addr, mem1.begin() + addr + buffer.size(),
                  buffer.begin());
    } else if (memIndex == 2) {
        std::copy(mem2.begin() + addr, mem2.begin() + addr + buffer.size(),
                  buffer.begin());
    } else {
        Logger::log("Memory", LogLevel::Error,
                    "readBlock: Invalid memory index (" +
                        std::to_string(memIndex) + ")");
    }
}

BigEndianStream Bus::readToStream(uint32_t addr, size_t count) {
    std::vector<uint8_t> buffer(count);
    readBlock(addr, buffer);
    return BigEndianStream(buffer);
}

MemoryStream::MemoryStream(uint32_t startAddress)
    : currentAddress(startAddress) {}

uint8_t MemoryStream::read8() {
    uint8_t value = Bus::read8(currentAddress);
    currentAddress += 1;
    return value;
}

uint16_t MemoryStream::read16() {
    uint16_t value = Bus::read16(currentAddress);
    currentAddress += 2;
    return value;
}

uint32_t MemoryStream::read32() {
    uint32_t value = Bus::read32(currentAddress);
    currentAddress += 4;
    return value;
}

uint64_t MemoryStream::read64() {
    uint64_t value = Bus::read64(currentAddress);
    currentAddress += 8;
    return value;
}
