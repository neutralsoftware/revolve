#ifndef VIDEO_INTERFACE_H
#define VIDEO_INTERFACE_H

#include "core/memory.h"
#include <array>
#include <cstdint>
#include <string>

constexpr uint32_t VI_BASE = 0x0C002000;
constexpr uint32_t VI_SIZE = 0x100;

class VideoInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;

    inline std::string getName() override { return "VI"; }

  private:
    std::array<uint16_t, VI_SIZE / 2> registers{};
};

#endif