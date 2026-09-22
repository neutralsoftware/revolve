#ifndef VIDEO_INTERFACE_H
#define VIDEO_INTERFACE_H

#include "core/memory.h"
#include <array>
#include <cstdint>
#include <string>

constexpr uint32_t VI_BASE = 0x0C002000;
constexpr uint32_t VI_SIZE = 0x100;

struct VIState {
    uint16_t verticalTiming = 0;
    uint16_t displayConfig = 0;

    uint32_t horizontalTiming0 = 0;
    uint32_t horizontalTiming1 = 0;

    uint32_t topFramebuffer = 0;
    uint32_t bottomFramebuffer = 0;

    uint16_t currentVerticalPosition = 1;
    uint16_t currentHorizontalPosition = 1;

    std::array<uint32_t, 4> displayInterrupts{};
};

enum class VIRegister : uint32_t {
    VerticalTiming = 0x00,
    Control = 0x02,

    HorizontalTiming0Hi = 0x04,
    HorizontalTiming0Lo = 0x06,
    HorizontalTiming1Hi = 0x08,
    HorizontalTiming1Lo = 0x0A,

    FbTopHi = 0x1C,
    FbTopLo = 0x1E,

    FbBottomHi = 0x24,
    FbBottomLo = 0x26,

    VerticalBeam = 0x2C,
    HorizontalBeam = 0x2E,

    Interupt0Hi = 0x30,
    Interupt0Lo = 0x32,
    Interupt1Hi = 0x34,
    Interupt1Lo = 0x36,
    Interupt2Hi = 0x38,
    Interupt2Lo = 0x3A,
    Interupt3Hi = 0x3C,
    Interupt3Lo = 0x3E,
};

static constexpr uint64_t VI_CYCLES_PER_FRAME = 12'160'000;
static constexpr uint32_t VI_TOTAL_LINES = 525;

static constexpr uint64_t VI_CYCLES_PER_LINE =
    VI_CYCLES_PER_FRAME / VI_TOTAL_LINES;

class VideoInterface : public MMIODevice {
  public:
    VideoInterface();

    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;

    std::string getName() override { return "VI"; }

    void initialize();
    void onScanLine();

  private:
    void decodeRegisterWrite(uint32_t offset, uint32_t value, AccessSize size);

    void checkInterrupts();

    uint32_t readRegister32(uint32_t offset) const;

    VIState state{};
    std::array<uint16_t, VI_SIZE / 2> registers{};
};

#endif