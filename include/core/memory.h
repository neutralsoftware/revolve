#ifndef REVOLVE_MEMORY
#define REVOLVE_MEMORY

#include "core/utils.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <sys/types.h>
#include <vector>

constexpr uint32_t MEM1_CACHED_START = 0x80000000;
constexpr uint32_t MEM1_CACHED_END = 0x817FFFFF;

constexpr uint32_t MEM1_UNCACHED_START = 0xC0000000;
constexpr uint32_t MEM1_UNCACHED_END = 0xC17FFFFF;

constexpr uint32_t MEM1_PHYS_START = 0x00000000;

constexpr uint32_t MEM2_CACHED_START = 0x90000000;
constexpr uint32_t MEM2_CACHED_END = 0x93FFFFFF;

constexpr uint32_t MEM2_UNCACHED_START = 0xD0000000;
constexpr uint32_t MEM2_UNCACHED_END = 0xD3FFFFFF;

constexpr uint32_t MEM2_PHYS_START = 0x10000000;

constexpr uint32_t MMIO_GAMECUBE_START = 0xCC000000;
constexpr uint32_t MMIO_GAMECUBE_END = 0xCC008003;
constexpr uint32_t MMIO_GAMECUBE_PHYS_START = 0x0C000000;
constexpr uint32_t MMIO_GAMECUBE_PHYS_END = 0x0C008003;

constexpr uint32_t MMIO_WII_START = 0xCD000000;
constexpr uint32_t MMIO_WII_END = 0xCD008000;
constexpr uint32_t MMIO_WII_PHYS_START = 0x0D000000;
constexpr uint32_t MMIO_WII_PHYS_END = 0x0D008000;

enum class MemoryRegion { MEM1, MEM2, MMIO, Invalid };

enum class MemoryAccess { Read, Write, Instruction };

struct MemoryAccessException {};

struct ResolvedAddress {
    MemoryRegion region;
    uint32_t offset;
};

class Memory {
  public:
    Memory() : mem1(MEM1_SIZE, 0), mem2(MEM2_SIZE, 0) {}

    void write8(uint32_t addr, uint8_t value, int memIndex = 1);
    void write16(uint32_t addr, uint16_t value, int memIndex = 1);
    void write32(uint32_t addr, uint32_t value, int memIndex = 1);
    void write64(uint32_t addr, uint64_t value, int memIndex = 1);

    void writeFloat(uint32_t addr, float value, int memIndex = 1);
    void writeDouble(uint32_t addr, double value, int memIndex = 1);

    uint8_t read8(uint32_t addr, int memIndex = 1);
    uint16_t read16(uint32_t addr, int memIndex = 1);
    uint32_t read32(uint32_t addr, int memIndex = 1);
    uint64_t read64(uint32_t addr, int memIndex = 1);

    float readFloat(uint32_t addr, int memIndex = 1);
    double readDouble(uint32_t addr, int memIndex = 1);

    void writeBlock(uint32_t addr, std::span<const uint8_t> bytes,
                    int memIndex = 1);
    void readBlock(uint32_t addr, std::vector<uint8_t> &buffer,
                   int memIndex = 1);

  private:
    static constexpr size_t MEM1_SIZE = 0x1800000;
    static constexpr size_t MEM2_SIZE = 0x4000000;

    std::vector<uint8_t> mem1;
    std::vector<uint8_t> mem2;
};

enum class AccessSize { U8, U16, U32, U64 };

class MMIODevice {
  public:
    virtual uint32_t read(uint32_t offset, AccessSize size) = 0;

    virtual void write(uint32_t offset, uint32_t value, AccessSize size) = 0;

    virtual std::string getName() = 0;
};

class MMIO {
  public:
    uint8_t read8(uint32_t addr);
    uint16_t read16(uint32_t addr);
    uint32_t read32(uint32_t addr);

    void write8(uint32_t addr, uint8_t value);
    void write16(uint32_t addr, uint16_t value);
    void write32(uint32_t addr, uint32_t value);

    void registerDevice(uint32_t baseAddr, uint32_t size, MMIODevice *device);

  private:
    struct MMIOEntry {
        uint32_t baseAddr;
        uint32_t size;
        MMIODevice *device;
    };

    std::vector<MMIOEntry> devices = {};
};

class Bus {
  public:
    static uint8_t read8(uint32_t addr);
    static uint16_t read16(uint32_t addr);
    static uint32_t read32(uint32_t addr);
    static uint64_t read64(uint32_t addr);
    static uint32_t fetch32(uint32_t addr);
    static uint32_t readPhysical32(uint32_t addr);
    static uint32_t readPhysical8(uint32_t addr);

    static float readFloat(uint32_t addr);
    static double readDouble(uint32_t addr);

    static void write8(uint32_t addr, uint8_t value);
    static void write16(uint32_t addr, uint16_t value);
    static void write32(uint32_t addr, uint32_t value);
    static void write64(uint32_t addr, uint64_t value);
    static void writePhysical32(uint32_t addr, uint32_t value);

    static void writeFloat(uint32_t addr, float value);
    static void writeDouble(uint32_t addr, double value);

    static void writeBlock(uint32_t addr, std::span<const uint8_t> bytes);
    static void readBlock(uint32_t addr, std::vector<uint8_t> &buffer);

    static void writeFromStream(uint32_t addr, size_t count,
                                BigEndianStream &stream);
    BigEndianStream readToStream(uint32_t addr, size_t count);

    static ResolvedAddress resolveAddress(uint32_t addr);
};

class MemoryStream {
  public:
    MemoryStream(uint32_t startAddress);

    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    uint64_t read64();

    float readFloat();
    double readDouble();

  private:
    uint32_t currentAddress;
};

#endif // REVOLVE_MEMORY
