#ifndef REVOLVE_BROADWAY
#define REVOLVE_BROADWAY

#include "core/memory.h"
#include "core/utils.h"
#include <cstdint>

struct BroadwayState : public Loggable {
    uint32_t gpr[32]{}; // General Purpose Registers

    uint64_t fpr[32]{}; // Floating Point Registers

    uint32_t pc = 0; // Program Counter

    uint32_t cr = 0;  // Condition Register
    uint32_t xer = 0; // Fixed-Point Exception Register
    uint32_t lr = 0;  // Link Register
    uint32_t ctr = 0; // Count Register

    uint32_t fpscr = 0; // Floating Point Status and Control Register

    uint32_t msr = 0; // Machine State Register

    uint32_t srr0 = 0;  // Save/Restore Register 0
    uint32_t srr1 = 0;  // Save/Restore Register 1
    uint32_t dar = 0;   // Data Address Register
    uint32_t dsisr = 0; // Data Storage Interrupt Status Register

    uint32_t dec = 0;      // Decrementer
    uint32_t timeBase = 0; // Time Base

    uint32_t sprg[4]{}; // Special Purpose Registers General
    uint32_t sr[16]{};  // Segment Registers
    uint32_t sdr1 = 0;  // Storage Description Register 1

    // Broadway / Gekko specific registers
    uint32_t gqr[8]{}; // Graphics Quantization Registers

    uint32_t hid0 = 0; // Hardware Implementation-Dependent Register 0
    uint32_t hid1 = 0; // Hardware Implementation-Dependent Register 1
    uint32_t hid2 = 0; // Hardware Implementation-Dependent Register 2
    uint32_t hid4 = 0; // Hardware Implementation-Dependent Register 4

    uint32_t wpar = 0; // Watchpoint Address Register
    uint32_t l2cr = 0; // L2 Cache Control Register

    uint32_t pvr = 0; // Processor Version Register

    std::string log() const override;
    std::string getLogSystem() const override { return "Broadway"; }
};

class Broadway {
  public:
    BroadwayState state{};

    void reset(uint32_t entryPoint);
    void executeInstruction();

    void start();

  private:
    MemoryStream instructionStream{0};
};

#endif