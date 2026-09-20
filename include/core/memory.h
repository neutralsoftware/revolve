#ifndef REVOLVE_MEMORY
#define REVOLVE_MEMORY

#include <array>
#include <cstdint>
#include <string>
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

constexpr uint32_t MMIO_WII_START = 0xCD000000;
constexpr uint32_t MMIO_WII_END = 0xCD008000;

enum class MemoryRegion { MEM1, MEM2, MMIO, Invalid };

struct ResolvedAddress {
    MemoryRegion region;
    uint32_t offset;
};

class Memory {
  public:
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

  private:
    std::array<uint8_t, 0x1800000> mem1;
    std::array<uint8_t, 0x4000000> mem2;
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

    static float readFloat(uint32_t addr);
    static double readDouble(uint32_t addr);

    void write8(uint32_t addr, uint8_t value);
    void write16(uint32_t addr, uint16_t value);
    void write32(uint32_t addr, uint32_t value);
    void write64(uint32_t addr, uint64_t value);

    void writeFloat(uint32_t addr, float value);
    void writeDouble(uint32_t addr, double value);

    static ResolvedAddress resolveAddress(uint32_t addr);
};

#endif // REVOLVE_MEMORY