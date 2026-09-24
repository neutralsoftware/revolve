#ifndef REVOLVE_SYSTEM_HARDWARE
#define REVOLVE_SYSTEM_HARDWARE

#include "core/memory.h"
#include <array>
#include <cstdint>
#include <string>

constexpr uint32_t DSP_BASE = 0x0C005000;
constexpr uint32_t DSP_SIZE = 0x40;
constexpr uint32_t SI_BASE = 0x0D006400;
constexpr uint32_t SI_SIZE = 0x100;
constexpr uint32_t EXI_BASE = 0x0D006800;
constexpr uint32_t EXI_SIZE = 0x40;

class DSPInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "DSPInterface"; }

  private:
    uint16_t read16(uint32_t offset);
    void write16(uint32_t offset, uint16_t value);
    void completeARAMTransfer();
    void updateInterrupt();

    uint32_t mailToDSP = 0;
    uint32_t mailFromDSP = 0;
    bool mailFromDSPReady = false;
    bool initCodeLoaded = false;
    uint16_t control = 0x0004;
    uint16_t interruptControl = 0;
    uint16_t arInfo = 0;
    uint16_t arRefresh = 156;
    uint32_t arMainAddress = 0;
    uint32_t arAddress = 0;
    uint32_t arCount = 0;
};

class SerialInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "SerialInterface"; }

  private:
    void updateInterrupt();
    std::array<uint32_t, SI_SIZE / 4> registers{};
};

class ExpansionInterface : public MMIODevice {
  public:
    ExpansionInterface();

    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "ExpansionInterface"; }

  private:
    void updateInterrupt();
    std::array<uint32_t, 3> status{};
    std::array<uint32_t, 3> dmaAddress{};
    std::array<uint32_t, 3> dmaLength{};
    std::array<uint32_t, 3> control{};
    std::array<uint32_t, 3> immediateData{};
};

#endif
