#ifndef REVOLVE_SYSTEM_HARDWARE
#define REVOLVE_SYSTEM_HARDWARE

#include "core/memory.h"
#include <array>
#include <cstdint>
#include <deque>
#include <string>

constexpr uint32_t DSP_BASE = 0x0C005000;
constexpr uint32_t DSP_SIZE = 0x40;
constexpr uint32_t DI_BASE = 0x0D006000;
constexpr uint32_t DI_SIZE = 0x40;
constexpr uint32_t SI_BASE = 0x0D006400;
constexpr uint32_t SI_SIZE = 0x100;
constexpr uint32_t EXI_BASE = 0x0D006800;
constexpr uint32_t EXI_SIZE = 0x40;

class DiscInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "DiscInterface"; }
};

class DSPInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "DSPInterface"; }
    void step(uint32_t cycles);

  private:
    uint16_t read16(uint32_t offset);
    void write16(uint32_t offset, uint16_t value);
    void completeARAMTransfer();
    void updateInterrupt();
    void queueDSPMail(uint32_t mail, bool interrupt);
    void handleDSPMail(uint32_t mail);
    void mixASND(bool clear);
    void writeASNDOutput();
    void mixAESND(bool clear);
    void writeAESNDOutput();

    uint32_t mailToDSP = 0;
    bool mailToDSPReady = false;
    uint32_t mailFromDSP = 0;
    bool mailFromDSPReady = false;
    std::deque<uint32_t> pendingDSPMails;
    uint32_t dspBootCommand = 0;
    uint32_t dspGeneration = 0;
    uint32_t asndVoiceAddress = 0;
    uint32_t asndOutputAddress = 0;
    uint32_t aesndParameterAddress = 0;
    uint32_t aesndOutputAddress = 0;
    uint8_t dspHLEState = 0;
    uint8_t dspUploadWords = 0;
    std::array<int16_t, 2048> asndOutput{};
    std::array<int16_t, 192> aesndOutput{};
    bool initCodeLoaded = false;
    uint16_t control = 0x0004;
    uint16_t interruptControl = 0;
    uint16_t arInfo = 0;
    uint16_t arRefresh = 156;
    uint32_t arMainAddress = 0;
    uint32_t arAddress = 0;
    uint32_t arCount = 0;
    uint32_t audioAddress = 0;
    uint32_t audioCursor = 0;
    uint16_t audioControl = 0;
    uint16_t audioBlocksLeft = 0;
    uint64_t audioAccumulator = 0;
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
