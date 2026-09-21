#ifndef REVOLVE_IPC
#define REVOLVE_IPC

#include "core/memory.h"
#include <cstdint>

constexpr uint32_t IPC_MMIO_BASE = 0x0D800000;
constexpr uint32_t IPC_MMIO_END = 0x0D80000F;

constexpr uint32_t IPC_PPCMSG = 0x0;
constexpr uint32_t IPC_PPCCTRL = 0x4;
constexpr uint32_t IPC_ARMMSG = 0x8;
constexpr uint32_t IPC_ARMCTRL = 0xC;

class IPC : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    inline std::string getName() override { return "IPC"; }

    void replyFromStarlet(uint32_t requestAddress);

    bool pccRequestPending() const;
    uint32_t getPPCMessage() const;

  private:
    uint32_t ppcMessage = 0; // Broadway -> Starlet message pointer
    uint32_t armMessage = 0; // Starlet -> Broadway response pointer

    bool x1 = false; // Broadway -> Starlet message ready
    bool x2 = false; // Relaunch
    bool y1 = false; // Starlet -> Broadway response ready
    bool y2 = false; // Starlet achnowledgement

    bool interruptY1 = false;
    bool interruptY2 = false;

    void updateInterrupts();
};

#endif