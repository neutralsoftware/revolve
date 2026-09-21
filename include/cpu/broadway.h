#ifndef REVOLVE_BROADWAY
#define REVOLVE_BROADWAY

#include "core/memory.h"
#include "core/utils.h"
#include <cstdint>

static constexpr uint32_t XER_SO = 1u << 31;
static constexpr uint32_t XER_OV = 1u << 30;
static constexpr uint32_t XER_CA = 1u << 29;

static constexpr uint32_t XER_BC_MASK = 0x7Fu;

enum SPR : uint16_t {
    XER = 1,
    LR = 8,
    CTR = 9,

    DSISR = 18,
    DAR = 19,
    DEC = 22,

    SDR1 = 25,
    SRR0 = 26,
    SRR1 = 27,

    SPRG0 = 272,
    SPRG1 = 273,
    SPRG2 = 274,
    SPRG3 = 275,

    PVR = 287,

    GQR0 = 912,
    GQR1 = 913,
    GQR2 = 914,
    GQR3 = 915,
    GQR4 = 916,
    GQR5 = 917,
    GQR6 = 918,
    GQR7 = 919,

    HID2 = 920,
    WPAR = 921,

    HID0 = 1008,
    HID1 = 1009,
    HID4 = 1011,
    DABR = 1013,
    L2CR = 1017
};

enum class BroadwayDTypeInstruction : uint32_t {
    // D-Type Instructions
    TWI = 3,
    MULLI = 7,
    SUBFIC = 8,
    CMPLI = 10,
    CMPI = 11,
    ADDIC = 12,
    ADDIC_ = 13,
    ADDI = 14,
    ADDIS = 15,
    ORI = 24,
    ORIS = 25,
    XORI = 26,
    XORIS = 27,
    ANDI_ = 28,
    ANDIS_ = 29,
    LWZ = 32,
    LWZU = 33,
    LBZ = 34,
    LBZU = 35,
    STW = 36,
    STWU = 37,
    STB = 38,
    STBU = 39,
    LHZ = 40,
    LHZU = 41,
    LHA = 42,
    LHAU = 43,
    STH = 44,
    STHU = 45,
    LMW = 46,
    STMW = 47,
    LFS = 48,
    LFSU = 49,
    LFD = 50,
    LFDU = 51,
    STFS = 52,
    STFSU = 53,
    STFD = 54,
    STFDU = 55,
};

enum class BroadwayMTypeInstruction : uint32_t {
    RLWIMI = 20,
    RLWINM = 21,
    RLWNM = 22,
};

enum class InstructionType {
    D,
    I,
    B,
    SC,
    X,
    XO,
    XL,
    XFX,
    XFL,
    M,
    A,
    PSQ_D,
    PSQ_X
};

struct BroadwayState : public Loggable {
    uint32_t gpr[32]{}; // General Purpose Registers

    uint64_t fpr[32]{}; // Floating Point Registers

    uint32_t cr{};    // Condition Register
    uint32_t fpscr{}; // Floating Point Status and Control Register
    uint32_t msr{};   // Machine State Register

    uint32_t sr[16]{}; // Segment Registers

    uint32_t spr[1024]{}; // Special Purpose Registers

    uint32_t cia{}; // Current Instruction Address
    uint32_t nia{}; // Next Instruction Address

    bool reservationValid{};       // Reservation Valid Bit
    uint32_t reservationAddress{}; // Reservation Address

    std::string log() const override;
    std::string getLogSystem() const override { return "Broadway"; }

    inline uint32_t &getSPR(SPR sprIndex) {
        return spr[static_cast<uint16_t>(sprIndex)];
    }

    inline bool getSO() {
        return (spr[static_cast<uint16_t>(SPR::XER)] & XER_SO) != 0;
    }

    inline bool getOV() {
        return (spr[static_cast<uint16_t>(SPR::XER)] & XER_OV) != 0;
    }

    inline bool getCA() {
        return (spr[static_cast<uint16_t>(SPR::XER)] & XER_CA) != 0;
    }

    inline void setSO(bool value) {
        if (value) {
            spr[static_cast<uint16_t>(SPR::XER)] |= XER_SO;
        } else {
            spr[static_cast<uint16_t>(SPR::XER)] &= ~XER_SO;
        }
    }

    inline void setOV(bool value) {
        if (value) {
            spr[static_cast<uint16_t>(SPR::XER)] |= XER_OV;
        } else {
            spr[static_cast<uint16_t>(SPR::XER)] &= ~XER_OV;
        }
    }

    inline void setCA(bool value) {
        if (value) {
            spr[static_cast<uint16_t>(SPR::XER)] |= XER_CA;
        } else {
            spr[static_cast<uint16_t>(SPR::XER)] &= ~XER_CA;
        }
    }

    inline void setCRField(uint32_t fieldIndex, uint32_t value) {
        uint32_t mask = 0b1111u << (28 - fieldIndex * 4);
        cr = (cr & ~mask) | ((value & 0b1111u) << (28 - fieldIndex * 4));
    }

    inline uint32_t getCRField(uint32_t fieldIndex) {
        return (cr >> (28 - fieldIndex * 4)) & 0b1111u;
    }

    inline void updateCR0(uint32_t in) {
        int32_t result = static_cast<int32_t>(in);
        uint32_t cr0 = 0;

        if (result < 0)
            cr0 |= 0b1000;
        else if (result > 0)
            cr0 |= 0b0100;
        else
            cr0 |= 0b0010;

        if (getSO())
            cr0 |= 0b0001;

        setCRField(0, cr0);
    }
};

constexpr int32_t signExtend(uint32_t x, unsigned bits) {
    const uint32_t sign = 1u << (bits - 1);
    return static_cast<int32_t>((x ^ sign) - sign);
}

class Broadway {
  public:
    BroadwayState state{};

    void reset(uint32_t entryPoint);
    void executeInstruction();

    void executeDType(uint32_t instruction);
    void executeIType(uint32_t instruction);
    void executeBType(uint32_t instruction);
    void executeSCType(uint32_t instruction);
    void executeXType(uint32_t instruction);
    void executeXOType(uint32_t instruction);
    void executeXLType(uint32_t instruction);
    void executeXFXType(uint32_t instruction);
    void executeXFLType(uint32_t instruction);
    void executeMType(uint32_t instruction);
    void executeAType(uint32_t instruction);
    void executePSQ_DType(uint32_t instruction);
    void executePSQ_XType(uint32_t instruction);

    InstructionType getInstructionType(uint32_t instruction);

    void start();

    // D Type instructions
    void executeTWI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeMULLI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSUBFIC(uint32_t field0, uint32_t field1, uint32_t,
                       int32_t simm);
    void executeCMPLI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeCMPI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeADDIC(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeADDIC_(uint32_t field0, uint32_t field1, uint32_t,
                       int32_t simm);
    void executeADDI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeADDIS(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeORI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeORIS(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeXORI(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeXORIS(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeANDI_(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeANDIS_(uint32_t field0, uint32_t field1, uint32_t,
                       int32_t simm);
    void executeLWZ(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLWZU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLBZ(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLBZU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTW(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTWU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTB(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTBU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLHZ(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLHZU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLHA(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLHAU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTH(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTHU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLMW(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTMW(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLFS(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLFSU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLFD(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeLFDU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTFS(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTFSU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTFD(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);
    void executeSTFDU(uint32_t field0, uint32_t field1, uint32_t, int32_t simm);

  private:
    MemoryStream instructionStream{0};
};

#endif