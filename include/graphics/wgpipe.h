#ifndef WGPIPE_H
#define WGPIPE_H

#include "core/memory.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

constexpr uint32_t WGPIPE_BASE = 0x0C008000;
constexpr uint32_t WGPIPE_SIZE = 0x20;

class WriteGatherPipe : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;

    std::string getName() override { return "Write Gather Pipe"; }

    bool empty() const { return count == 0; }

  private:
    void push8(uint8_t value);
    void push16(uint16_t value);
    void push32(uint32_t value);

    void flush();

    std::array<uint8_t, WGPIPE_SIZE> buffer;
    size_t count = 0;
};

#endif