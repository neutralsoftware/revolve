#include "core/utils.h"
#include "cpu/broadway.h"
#include <bit>

void Broadway::executeTWI(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t to = field0;
    uint32_t ra = field1;

    uint32_t a = state.gpr[ra];

    int32_t signedA = static_cast<int32_t>(a);
    uint32_t unsignedImmediate = static_cast<uint32_t>(simm);

    bool trap = ((to & 0x10) && signedA < simm) ||
                ((to & 0x08) && signedA > simm) ||
                ((to & 0x04) && signedA == simm) ||
                ((to & 0x02) && a < unsignedImmediate) ||
                ((to & 0x01) && a > unsignedImmediate);

    if (trap) {
        raiseException(0x700, 0x20000);
    }
}

void Broadway::executeMULLI(uint32_t field0, uint32_t field1, uint32_t,
                            int32_t simm) {
    int32_t a = static_cast<int32_t>(state.gpr[field1]);

    int64_t result = static_cast<int64_t>(a) * static_cast<int64_t>(simm);

    state.gpr[field0] = static_cast<uint32_t>(result);
}

void Broadway::executeSUBFIC(uint32_t field0, uint32_t field1, uint32_t,
                             int32_t simm) {
    uint32_t a = state.gpr[field1];
    uint32_t imm = static_cast<uint32_t>(simm);

    state.gpr[field0] = imm - a;

    state.setCA(imm >= a);
}

void Broadway::executeCMPLI(uint32_t field0, uint32_t field1, uint32_t uimm,
                            int32_t) {
    uint32_t bf = field0 >> 2;
    uint32_t ra = field1;

    uint32_t a = state.gpr[ra];

    state.setCRField(bf, (a < uimm ? 8u : a > uimm ? 4u : 2u) | state.getSO());
}

void Broadway::executeCMPI(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t bf = field0 >> 2;
    uint32_t ra = field1;

    int32_t a = static_cast<int32_t>(state.gpr[ra]);

    state.setCRField(bf, (a < simm ? 8u : a > simm ? 4u : 2u) | state.getSO());
}

void Broadway::executeADDIC(uint32_t field0, uint32_t field1, uint32_t,
                            int32_t simm) {
    uint32_t a = state.gpr[field1];
    uint32_t imm = static_cast<uint32_t>(simm);

    uint64_t result = static_cast<uint64_t>(a) + static_cast<uint64_t>(imm);

    state.gpr[field0] = static_cast<uint32_t>(result);

    state.setCA((result >> 32) != 0);
}

void Broadway::executeADDIC_(uint32_t field0, uint32_t field1, uint32_t,
                             int32_t simm) {
    uint32_t a = state.gpr[field1];
    uint32_t imm = static_cast<uint32_t>(simm);

    uint64_t result64 = static_cast<uint64_t>(a) + static_cast<uint64_t>(imm);

    uint32_t result = static_cast<uint32_t>(result64);

    state.gpr[field0] = result;
    state.setCA((result64 >> 32) != 0);

    state.updateCR0(result);
}

void Broadway::executeADDI(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    state.gpr[field0] = a + static_cast<uint32_t>(simm);
}

void Broadway::executeADDIS(uint32_t field0, uint32_t field1, uint32_t,
                            int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    state.gpr[field0] = a + (static_cast<uint32_t>(simm) << 16);
}

void Broadway::executeORI(uint32_t field0, uint32_t field1, uint32_t uimm,
                          int32_t) {
    uint32_t result = state.gpr[field0] | uimm;
    state.gpr[field1] = result;
}

void Broadway::executeORIS(uint32_t field0, uint32_t field1, uint32_t uimm,
                           int32_t) {
    uint32_t result = state.gpr[field0] | (uimm << 16);
    state.gpr[field1] = result;
}

void Broadway::executeXORI(uint32_t field0, uint32_t field1, uint32_t uimm,
                           int32_t) {
    uint32_t result = state.gpr[field0] ^ uimm;
    state.gpr[field1] = result;
}

void Broadway::executeXORIS(uint32_t field0, uint32_t field1, uint32_t uimm,
                            int32_t) {
    uint32_t result = state.gpr[field0] ^ (uimm << 16);
    state.gpr[field1] = result;
}

void Broadway::executeANDI_(uint32_t field0, uint32_t field1, uint32_t uimm,
                            int32_t) {
    uint32_t result = state.gpr[field0] & uimm;
    state.gpr[field1] = result;
    state.updateCR0(result);
}

void Broadway::executeANDIS_(uint32_t field0, uint32_t field1, uint32_t uimm,
                             int32_t) {
    uint32_t result = state.gpr[field0] & (uimm << 16);
    state.gpr[field1] = result;
    state.updateCR0(result);
}

void Broadway::executeLWZ(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    state.gpr[field0] = Bus::read32(effectiveAddress);
}

void Broadway::executeLWZU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0 || ra == rd) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    uint32_t value = Bus::read32(effectiveAddress);

    state.gpr[rd] = value;
    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeSTW(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    Bus::write32(effectiveAddress, state.gpr[field0]);
}

void Broadway::executeSTWU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    Bus::write32(effectiveAddress, state.gpr[rd]);

    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeSTB(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    Bus::write8(effectiveAddress, static_cast<uint8_t>(state.gpr[field0]));
}

void Broadway::executeSTBU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    Bus::write8(effectiveAddress, static_cast<uint8_t>(state.gpr[rd]));

    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeLHZ(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    state.gpr[field0] = Bus::read16(effectiveAddress);
}

void Broadway::executeLHZU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0 || ra == rd) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    state.gpr[rd] = Bus::read16(effectiveAddress);
    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeLHA(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    uint16_t halfword = Bus::read16(effectiveAddress);
    state.gpr[field0] = signExtend(halfword, 16);
}

void Broadway::executeLHAU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0 || ra == rd) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    uint16_t halfword = Bus::read16(effectiveAddress);
    state.gpr[rd] = signExtend(halfword, 16);
    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeSTH(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    Bus::write16(effectiveAddress, static_cast<uint16_t>(state.gpr[field0]));
}

void Broadway::executeSTHU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    Bus::write16(effectiveAddress, static_cast<uint16_t>(state.gpr[rd]));

    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeLMW(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    for (uint32_t i = 0; i < 32 - field0; ++i) {
        state.gpr[field0 + i] = Bus::read32(effectiveAddress + i * 4);
    }
}

void Broadway::executeSTMW(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    for (uint32_t i = 0; i < 32 - field0; ++i) {
        Bus::write32(effectiveAddress + i * 4, state.gpr[field0 + i]);
    }
}

void Broadway::executeLFS(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    uint32_t value = Bus::read32(effectiveAddress);
    state.fpr[field0] = std::bit_cast<uint64_t>(
        static_cast<double>(std::bit_cast<float>(value)));
    state.ps1[field0] = state.fpr[field0];
}

void Broadway::executeLFSU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    uint32_t value = Bus::read32(effectiveAddress);
    state.fpr[rd] = std::bit_cast<uint64_t>(
        static_cast<double>(std::bit_cast<float>(value)));
    state.ps1[rd] = state.fpr[rd];
    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeLFD(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    uint64_t value = Bus::read64(effectiveAddress);
    state.fpr[field0] = value;
}

void Broadway::executeLFDU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    uint64_t value = Bus::read64(effectiveAddress);
    state.fpr[rd] = value;
    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeSTFS(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    uint32_t value = singleStoreBits(state.fpr[field0]);
    Bus::write32(effectiveAddress, value);
}

void Broadway::executeSTFSU(uint32_t field0, uint32_t field1, uint32_t,
                            int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    uint32_t value = singleStoreBits(state.fpr[rd]);
    Bus::write32(effectiveAddress, value);

    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeSTFD(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    uint32_t ra = field1;

    uint32_t a = (ra == 0) ? 0 : state.gpr[ra];
    uint32_t effectiveAddress = a + static_cast<uint32_t>(simm);

    uint64_t value = state.fpr[field0];
    Bus::write64(effectiveAddress, value);
}

void Broadway::executeSTFDU(uint32_t field0, uint32_t field1, uint32_t,
                            int32_t simm) {
    uint32_t rd = field0;
    uint32_t ra = field1;

    if (ra == 0) {
        raiseException(0x700, 0x80000);
        return;
    }

    uint32_t effectiveAddress = state.gpr[ra] + static_cast<uint32_t>(simm);

    uint64_t value = state.fpr[rd];
    Bus::write64(effectiveAddress, value);

    state.gpr[ra] = effectiveAddress;
}

void Broadway::executeLBZ(uint32_t field0, uint32_t field1, uint32_t,
                          int32_t simm) {
    uint32_t address =
        (field1 == 0 ? 0 : state.gpr[field1]) + static_cast<uint32_t>(simm);
    state.gpr[field0] = Bus::read8(address);
}

void Broadway::executeLBZU(uint32_t field0, uint32_t field1, uint32_t,
                           int32_t simm) {
    if (field1 == 0 || field1 == field0) {
        raiseException(0x700, 0x80000);
        return;
    }
    uint32_t address = state.gpr[field1] + static_cast<uint32_t>(simm);
    state.gpr[field0] = Bus::read8(address);
    state.gpr[field1] = address;
}
