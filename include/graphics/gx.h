#ifndef GX_H
#define GX_H

#include <array>
#include <cstdint>

struct GXFifoReader {
    uint32_t cursor = 0;
    uint32_t bytesIntoBlock = 0;
    uint32_t availableBytes = 0;
};

enum class GXCommand : uint8_t {
    NOP = 0x00,
    CPLoad = 0x08,
    XFLoad = 0x10,
    BPLoad = 0x61
};

struct GXCPState {
    std::array<uint32_t, 256> registers{};
};

struct GXXFState {
    std::array<uint32_t, 0x2000> registers{};
};

struct GXBPState {
    std::array<uint32_t, 256> registers{};
};

struct GXState {
    GXCPState cp{};
    GXXFState xf{};
    GXBPState bp{};
};

class GX {
  public:
    void run();
    void processCommand();

    void initializeFifoReader();

  private:
    uint8_t read8();
    uint16_t read16();
    uint32_t read32();

    void processCPLoad();
    void processXFLoad();
    void processBPLoad();

    void processPrimitive(uint8_t command);

    GXFifoReader reader{};
    GXState state{};
};

#endif