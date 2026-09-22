#ifndef GX_H
#define GX_H

#include <cstdint>

struct GXFifoReader {
    uint32_t cursor = 0;
    uint32_t bytesIntoBlock = 0;
};

enum class GXCommand : uint8_t {
    NOP = 0x00,
    CPLoad = 0x08,
    XFLoad = 0x10,
    BPLoad = 0x61
};

class GX {
  public:
    void run();

  private:
    uint8_t read8();
    uint16_t read16();
    uint32_t read32();

    void processCommand();

    GXFifoReader reader{};
};

#endif