#include "cpu/broadway.h"
#include "device.h"
#include "core/utils.h"
#include <bit>
#include <cstdint>
#include <string>

void Broadway::reset(uint32_t entryPoint) {
    state = {};
    invalidateTranslationCache();
    timeBaseRemainder = 0;
    state.cia = entryPoint;
    state.nia = entryPoint + 4;
    state.spr[SPR::DEC] = 0xFFFFFFFF;
    setupWiiHLEBootState();
}

std::string BroadwayState::log() const {
    std::string logMessage = "Broadway State:\n";
    logMessage += "CIA: 0x" + utils::toHexString(cia) + "\n";
    logMessage += "NIA: 0x" + utils::toHexString(nia) + "\n";
    logMessage += "MSR: 0x" + utils::toHexString(msr) + "\n";
    logMessage += "CR: 0x" + utils::toHexString(cr) + "\n";
    logMessage += "FPSCR: 0x" + utils::toHexString(fpscr) + "\n";
    logMessage += "GPRs:\n";
    for (int i = 0; i < 32; ++i) {
        logMessage += "  R" + std::to_string(i) + ": 0x" +
                      utils::toHexString(gpr[i]) + "\n";
    }
    logMessage += "FPRs:\n";
    for (int i = 0; i < 32; ++i) {
        logMessage += "  F" + std::to_string(i) + ": 0x" +
                      utils::toHexString(static_cast<uint32_t>(fpr[i])) + "\n";
    }
    return logMessage;
}

void Broadway::start() {
    while (true) {
        executeInstruction();
    }
}

InstructionType Broadway::getInstructionType(uint32_t instruction) {
    uint32_t op = instruction >> 26;
    switch (static_cast<BroadwayDTypeInstruction>(op)) {
    case BroadwayDTypeInstruction::TWI:
    case BroadwayDTypeInstruction::MULLI:
    case BroadwayDTypeInstruction::SUBFIC:
    case BroadwayDTypeInstruction::CMPLI:
    case BroadwayDTypeInstruction::CMPI:
    case BroadwayDTypeInstruction::ADDIC:
    case BroadwayDTypeInstruction::ADDIC_:
    case BroadwayDTypeInstruction::ADDI:
    case BroadwayDTypeInstruction::ADDIS:
    case BroadwayDTypeInstruction::ORI:
    case BroadwayDTypeInstruction::ORIS:
    case BroadwayDTypeInstruction::XORI:
    case BroadwayDTypeInstruction::XORIS:
    case BroadwayDTypeInstruction::ANDI_:
    case BroadwayDTypeInstruction::ANDIS_:
    case BroadwayDTypeInstruction::LWZ:
    case BroadwayDTypeInstruction::LWZU:
    case BroadwayDTypeInstruction::STW:
    case BroadwayDTypeInstruction::STWU:
    case BroadwayDTypeInstruction::STB:
    case BroadwayDTypeInstruction::STBU:
    case BroadwayDTypeInstruction::LHZ:
    case BroadwayDTypeInstruction::LHZU:
    case BroadwayDTypeInstruction::LHA:
    case BroadwayDTypeInstruction::LHAU:
    case BroadwayDTypeInstruction::STH:
    case BroadwayDTypeInstruction::STHU:
    case BroadwayDTypeInstruction::LMW:
    case BroadwayDTypeInstruction::STMW:
    case BroadwayDTypeInstruction::LFS:
    case BroadwayDTypeInstruction::LFSU:
    case BroadwayDTypeInstruction::LFD:
    case BroadwayDTypeInstruction::LFDU:
    case BroadwayDTypeInstruction::STFS:
    case BroadwayDTypeInstruction::STFSU:
    case BroadwayDTypeInstruction::STFD:
    case BroadwayDTypeInstruction::STFDU:
    case BroadwayDTypeInstruction::LBZ:
    case BroadwayDTypeInstruction::LBZU:
        return InstructionType::D;
    }

    switch (static_cast<BroadwayMTypeInstruction>(op)) {
    case BroadwayMTypeInstruction::RLWIMI:
    case BroadwayMTypeInstruction::RLWINM:
    case BroadwayMTypeInstruction::RLWNM:
        return InstructionType::M;
    }

    if (op == 18) {
        return InstructionType::I;
    } else if (op == 16) {
        return InstructionType::B;
    }

    if (op == 17)
        return InstructionType::SC;
    if (op == 19)
        return InstructionType::XL;
    if (op == 56 || op == 57 || op == 60 || op == 61)
        return InstructionType::PSQ_D;
    if (op == 4)
        return InstructionType::PSQ_X;
    if (op == 59 || op == 63)
        return InstructionType::A;
    if (op == 31) {
        uint32_t xo = (instruction >> 1) & 0x3FF;
        switch (xo & 0x1FF) {
        case 266:
        case 10:
        case 138:
        case 234:
        case 202:
        case 491:
        case 459:
        case 235:
        case 104:
        case 40:
        case 8:
        case 136:
        case 232:
        case 200:
            return InstructionType::XO;
        }
        if (xo == 75 || xo == 11)
            return InstructionType::XO;
        if (xo == 19 || xo == 144 || xo == 339 || xo == 467 || xo == 371)
            return InstructionType::XFX;
    }
    return InstructionType::X;
}

Broadway::InstructionExecutor Broadway::executorFor(InstructionType type) {
    switch (type) {
    case InstructionType::D:
        return &Broadway::executeDType;
    case InstructionType::I:
        return &Broadway::executeIType;
    case InstructionType::B:
        return &Broadway::executeBType;
    case InstructionType::SC:
        return &Broadway::executeSCType;
    case InstructionType::X:
        return &Broadway::executeXType;
    case InstructionType::XO:
        return &Broadway::executeXOType;
    case InstructionType::XFX:
        return &Broadway::executeXFXType;
    case InstructionType::XFL:
        return &Broadway::executeXFLType;
    case InstructionType::XL:
        return &Broadway::executeXLType;
    case InstructionType::M:
        return &Broadway::executeMType;
    case InstructionType::A:
        return &Broadway::executeAType;
    case InstructionType::PSQ_D:
        return &Broadway::executePSQ_DType;
    case InstructionType::PSQ_X:
        return &Broadway::executePSQ_XType;
    }
    return &Broadway::executeXType;
}

void Broadway::compileInstruction(DecodedInstruction &entry) {
    const uint32_t instruction = entry.instruction;
    const uint32_t op = instruction >> 26;
    entry.field0 = (instruction >> 21) & 31;
    entry.field1 = (instruction >> 16) & 31;
    entry.field2 = (instruction >> 11) & 31;
    entry.immediate = instruction & 0xFFFF;
    entry.auxiliary = 0;
    entry.operation = CompiledOperation::Generic;
    switch (op) {
    case 7:
        entry.operation = CompiledOperation::MULLI;
        break;
    case 10:
        entry.operation = CompiledOperation::CMPLI;
        break;
    case 11:
        entry.operation = CompiledOperation::CMPI;
        break;
    case 12:
        entry.operation = CompiledOperation::ADDIC;
        break;
    case 13:
        entry.operation = CompiledOperation::ADDICRecord;
        break;
    case 14:
        entry.operation = CompiledOperation::ADDI;
        break;
    case 15:
        entry.operation = CompiledOperation::ADDIS;
        break;
    case 16:
        entry.operation = CompiledOperation::ConditionalBranch;
        entry.immediate = instruction & 0xFFFC;
        entry.auxiliary = instruction & 3;
        break;
    case 18:
        entry.operation = CompiledOperation::Branch;
        entry.immediate = instruction & 0x03FFFFFC;
        entry.auxiliary = instruction & 3;
        break;
    case 21:
        entry.operation = CompiledOperation::RLWINM;
        entry.immediate = (instruction >> 11) & 31;
        entry.auxiliary = utils::makeMask((instruction >> 6) & 31,
                                          (instruction >> 1) & 31);
        break;
    case 24:
        entry.operation = CompiledOperation::ORI;
        break;
    case 25:
        entry.operation = CompiledOperation::ORIS;
        break;
    case 26:
        entry.operation = CompiledOperation::XORI;
        break;
    case 27:
        entry.operation = CompiledOperation::XORIS;
        break;
    case 28:
        entry.operation = CompiledOperation::ANDI;
        break;
    case 29:
        entry.operation = CompiledOperation::ANDIS;
        break;
    case 31: {
        const uint32_t xo = (instruction >> 1) & 1023;
        if (xo == 28)
            entry.operation = CompiledOperation::AND;
        else if (xo == 444)
            entry.operation = CompiledOperation::OR;
        else if (xo == 316)
            entry.operation = CompiledOperation::XOR;
        else if (xo == 266 && !(instruction & 0x400))
            entry.operation = CompiledOperation::ADD;
        else if (xo == 40 && !(instruction & 0x400))
            entry.operation = CompiledOperation::SUBF;
        else if (xo == 0)
            entry.operation = CompiledOperation::CMP;
        else if (xo == 32)
            entry.operation = CompiledOperation::CMPL;
        break;
    }
    case 32:
        entry.operation = CompiledOperation::LWZ;
        break;
    case 33:
        if (entry.field1 != 0 && entry.field1 != entry.field0)
            entry.operation = CompiledOperation::LWZU;
        break;
    case 34:
        entry.operation = CompiledOperation::LBZ;
        break;
    case 35:
        if (entry.field1 != 0 && entry.field1 != entry.field0)
            entry.operation = CompiledOperation::LBZU;
        break;
    case 36:
        entry.operation = CompiledOperation::STW;
        break;
    case 37:
        if (entry.field1 != 0)
            entry.operation = CompiledOperation::STWU;
        break;
    case 38:
        entry.operation = CompiledOperation::STB;
        break;
    case 39:
        if (entry.field1 != 0)
            entry.operation = CompiledOperation::STBU;
        break;
    case 40:
        entry.operation = CompiledOperation::LHZ;
        break;
    case 41:
        if (entry.field1 != 0 && entry.field1 != entry.field0)
            entry.operation = CompiledOperation::LHZU;
        break;
    case 42:
        entry.operation = CompiledOperation::LHA;
        break;
    case 43:
        if (entry.field1 != 0 && entry.field1 != entry.field0)
            entry.operation = CompiledOperation::LHAU;
        break;
    case 44:
        entry.operation = CompiledOperation::STH;
        break;
    case 45:
        if (entry.field1 != 0)
            entry.operation = CompiledOperation::STHU;
        break;
    default:
        break;
    }
}

void Broadway::executeCompiled(const DecodedInstruction &entry) {
    const uint32_t signedImmediate = static_cast<uint32_t>(
        signExtend(entry.immediate, 16));
    switch (entry.operation) {
    case CompiledOperation::ADDI:
        state.gpr[entry.field0] =
            (entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
            signedImmediate;
        return;
    case CompiledOperation::ADDIS:
        state.gpr[entry.field0] =
            (entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
            (signedImmediate << 16);
        return;
    case CompiledOperation::ORI:
        state.gpr[entry.field1] =
            state.gpr[entry.field0] | entry.immediate;
        return;
    case CompiledOperation::ORIS:
        state.gpr[entry.field1] =
            state.gpr[entry.field0] | (entry.immediate << 16);
        return;
    case CompiledOperation::XORI:
        state.gpr[entry.field1] =
            state.gpr[entry.field0] ^ entry.immediate;
        return;
    case CompiledOperation::XORIS:
        state.gpr[entry.field1] =
            state.gpr[entry.field0] ^ (entry.immediate << 16);
        return;
    case CompiledOperation::ANDI: {
        const uint32_t result = state.gpr[entry.field0] & entry.immediate;
        state.gpr[entry.field1] = result;
        state.updateCR0(result);
        return;
    }
    case CompiledOperation::ANDIS: {
        const uint32_t result =
            state.gpr[entry.field0] & (entry.immediate << 16);
        state.gpr[entry.field1] = result;
        state.updateCR0(result);
        return;
    }
    case CompiledOperation::MULLI:
        state.gpr[entry.field0] = static_cast<uint32_t>(
            static_cast<int64_t>(static_cast<int32_t>(state.gpr[entry.field1])) *
            static_cast<int32_t>(signedImmediate));
        return;
    case CompiledOperation::ADDIC:
    case CompiledOperation::ADDICRecord: {
        const uint64_t result = static_cast<uint64_t>(state.gpr[entry.field1]) +
                                signedImmediate;
        state.gpr[entry.field0] = static_cast<uint32_t>(result);
        state.setCA((result >> 32) != 0);
        if (entry.operation == CompiledOperation::ADDICRecord)
            state.updateCR0(state.gpr[entry.field0]);
        return;
    }
    case CompiledOperation::LWZ:
        state.gpr[entry.field0] = Bus::read32(
            (entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
            signedImmediate);
        return;
    case CompiledOperation::LWZU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        state.gpr[entry.field0] = Bus::read32(address);
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::STW:
        Bus::write32((entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
                         signedImmediate,
                     state.gpr[entry.field0]);
        return;
    case CompiledOperation::STWU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        Bus::write32(address, state.gpr[entry.field0]);
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::LBZ:
        state.gpr[entry.field0] = Bus::read8(
            (entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
            signedImmediate);
        return;
    case CompiledOperation::LBZU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        state.gpr[entry.field0] = Bus::read8(address);
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::STB:
        Bus::write8((entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
                        signedImmediate,
                    static_cast<uint8_t>(state.gpr[entry.field0]));
        return;
    case CompiledOperation::STBU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        Bus::write8(address, static_cast<uint8_t>(state.gpr[entry.field0]));
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::LHZ:
        state.gpr[entry.field0] = Bus::read16(
            (entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
            signedImmediate);
        return;
    case CompiledOperation::LHZU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        state.gpr[entry.field0] = Bus::read16(address);
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::LHA:
        state.gpr[entry.field0] = static_cast<uint32_t>(signExtend(
            Bus::read16((entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
                        signedImmediate),
            16));
        return;
    case CompiledOperation::LHAU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        state.gpr[entry.field0] = static_cast<uint32_t>(
            signExtend(Bus::read16(address), 16));
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::STH:
        Bus::write16((entry.field1 == 0 ? 0 : state.gpr[entry.field1]) +
                         signedImmediate,
                     static_cast<uint16_t>(state.gpr[entry.field0]));
        return;
    case CompiledOperation::STHU: {
        const uint32_t address = state.gpr[entry.field1] + signedImmediate;
        Bus::write16(address,
                     static_cast<uint16_t>(state.gpr[entry.field0]));
        state.gpr[entry.field1] = address;
        return;
    }
    case CompiledOperation::CMPI: {
        const int32_t left = static_cast<int32_t>(state.gpr[entry.field1]);
        const int32_t right = static_cast<int32_t>(signedImmediate);
        state.setCRField(entry.field0 >> 2,
                         (left < right ? 8u : left > right ? 4u : 2u) |
                             state.getSO());
        return;
    }
    case CompiledOperation::CMPLI: {
        const uint32_t left = state.gpr[entry.field1];
        state.setCRField(entry.field0 >> 2,
                         (left < entry.immediate
                              ? 8u
                              : left > entry.immediate ? 4u : 2u) |
                             state.getSO());
        return;
    }
    case CompiledOperation::AND:
    case CompiledOperation::OR:
    case CompiledOperation::XOR: {
        const uint32_t left = state.gpr[entry.field0];
        const uint32_t right = state.gpr[entry.field2];
        const uint32_t result =
            entry.operation == CompiledOperation::AND
                ? left & right
                : entry.operation == CompiledOperation::OR ? left | right
                                                           : left ^ right;
        state.gpr[entry.field1] = result;
        if (entry.instruction & 1)
            state.updateCR0(result);
        return;
    }
    case CompiledOperation::ADD:
    case CompiledOperation::SUBF: {
        const uint32_t left = state.gpr[entry.field1];
        const uint32_t right = state.gpr[entry.field2];
        const uint32_t result = entry.operation == CompiledOperation::ADD
                                    ? left + right
                                    : right - left;
        state.gpr[entry.field0] = result;
        if (entry.instruction & 1)
            state.updateCR0(result);
        return;
    }
    case CompiledOperation::CMP:
    case CompiledOperation::CMPL: {
        const uint32_t left = state.gpr[entry.field1];
        const uint32_t right = state.gpr[entry.field2];
        uint32_t value = 0;
        if (entry.operation == CompiledOperation::CMP) {
            const int32_t signedLeft = static_cast<int32_t>(left);
            const int32_t signedRight = static_cast<int32_t>(right);
            value = signedLeft < signedRight
                        ? 8u
                        : signedLeft > signedRight ? 4u : 2u;
        } else {
            value = left < right ? 8u : left > right ? 4u : 2u;
        }
        state.setCRField(entry.field0 >> 2, value | state.getSO());
        return;
    }
    case CompiledOperation::Branch: {
        const uint32_t displacement = static_cast<uint32_t>(
            signExtend(entry.immediate, 26));
        state.nia = (entry.auxiliary & 2) ? displacement
                                         : state.cia + displacement;
        if (entry.auxiliary & 1)
            state.spr[SPR::LR] = state.cia + 4;
        return;
    }
    case CompiledOperation::ConditionalBranch: {
        const uint32_t bo = entry.field0;
        const uint32_t bi = entry.field1;
        const uint32_t displacement = static_cast<uint32_t>(
            signExtend(entry.immediate, 16));
        if (branchCondition(bo, bi, true))
            state.nia = (entry.auxiliary & 2) ? displacement
                                             : state.cia + displacement;
        if (entry.auxiliary & 1)
            state.spr[SPR::LR] = state.cia + 4;
        return;
    }
    case CompiledOperation::RLWINM: {
        const uint32_t result =
            std::rotl(state.gpr[entry.field0], int(entry.immediate)) &
            entry.auxiliary;
        state.gpr[entry.field1] = result;
        if (entry.instruction & 1)
            state.updateCR0(result);
        return;
    }
    case CompiledOperation::Generic:
        (this->*entry.executor)(entry.instruction);
        return;
    }
}

void Broadway::executeDType(uint32_t instruction) {
    uint32_t op = instruction >> 26;

    uint32_t field0 = (instruction >> 21) & 0x1F;
    uint32_t field1 = (instruction >> 16) & 0x1F;

    uint32_t uimm = instruction & 0xFFFF;
    int32_t simm = signExtend(uimm, 16);

    // Note some instructions represent A = 0 as "no register" and should be
    // handled accordingly.
    BroadwayDTypeInstruction dtypeInstruction =
        static_cast<BroadwayDTypeInstruction>(op);
    switch (dtypeInstruction) {
    case BroadwayDTypeInstruction::TWI:
        executeTWI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::MULLI:
        executeMULLI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::SUBFIC:
        executeSUBFIC(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::CMPLI:
        executeCMPLI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::CMPI:
        executeCMPI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ADDIC:
        executeADDIC(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ADDIC_:
        executeADDIC_(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ADDI:
        executeADDI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ADDIS:
        executeADDIS(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ORI:
        executeORI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ORIS:
        executeORIS(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::XORI:
        executeXORI(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::XORIS:
        executeXORIS(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ANDI_:
        executeANDI_(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::ANDIS_:
        executeANDIS_(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LWZ:
        executeLWZ(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LWZU:
        executeLWZU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STW:
        executeSTW(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STWU:
        executeSTWU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STB:
        executeSTB(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STBU:
        executeSTBU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LHZ:
        executeLHZ(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LHZU:
        executeLHZU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LHA:
        executeLHA(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LHAU:
        executeLHAU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STH:
        executeSTH(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STHU:
        executeSTHU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LMW:
        executeLMW(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STMW:
        executeSTMW(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LFS:
        executeLFS(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LFSU:
        executeLFSU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LFD:
        executeLFD(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LFDU:
        executeLFDU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STFS:
        executeSTFS(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STFSU:
        executeSTFSU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STFD:
        executeSTFD(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::STFDU:
        executeSTFDU(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LBZ:
        executeLBZ(field0, field1, uimm, simm);
        break;
    case BroadwayDTypeInstruction::LBZU:
        executeLBZU(field0, field1, uimm, simm);
        break;
    default:
        Logger::log("Broadway", LogLevel::Error,
                    "executeDType: Unknown D-Type instruction. CIA: " +
                        utils::toHexString(state.cia));
        break;
    }
}

void Broadway::executeIType(uint32_t instruction) {
    int32_t displacement = signExtend(instruction & 0x03FFFFFC, 26);

    bool aa = (instruction >> 1) & 0x1;
    bool lk = instruction & 0x1;

    uint32_t targetAddress =
        aa ? static_cast<uint32_t>(displacement)
           : state.cia + static_cast<uint32_t>(displacement);

    if (lk) {
        state.spr[SPR::LR] = state.cia + 4;
    }

    state.nia = targetAddress;
}

void Broadway::raiseException(uint32_t vector, uint32_t cause) {
    state.spr[SPR::SRR0] = state.cia;
    state.spr[SPR::SRR1] = (state.msr & 0x87C0FFFFu) | cause;
    state.nia = ((state.msr & 0x40) ? 0xFFF00000u : 0) | vector;
    state.msr = (state.msr & ~0x0004EF37u) | ((state.msr >> 16) & 1);
    state.reservationValid = false;
    state.exceptionTaken = true;
}

void Broadway::requestExternalInterrupt() {
    state.externalInterruptPending = true;
}

void Broadway::clearExternalInterrupt() {
    state.externalInterruptPending = false;
}

void Broadway::requestSystemReset() { state.systemResetPending = true; }

void Broadway::requestMachineCheck() { state.machineCheckPending = true; }

bool Broadway::deliverPendingException() {
    if (state.systemResetPending) {
        state.systemResetPending = false;
        raiseException(0x100);
        return true;
    }
    if (state.machineCheckPending && (state.msr & 0x1000)) {
        state.machineCheckPending = false;
        raiseException(0x200);
        state.msr &= ~0x1000u; // Clear ME bit
        return true;
    }
    if (!(state.msr & 0x8000))
        return false;
    if (state.externalInterruptPending) {
        raiseException(0x500);
        return true;
    }
    if (state.decrementerPending) {
        state.decrementerPending = false;
        raiseException(0x900);
        return true;
    }
    return false;
}

bool Broadway::protectionAllows(uint32_t protection, bool key,
                                MemoryAccess access) {
    if (access == MemoryAccess::Instruction)
        access = MemoryAccess::Read;
    switch (protection) {
    case 0:
        return !key;
    case 1:
        return access == MemoryAccess::Read || !key;
    case 2:
        return true;
    case 3:
        return access == MemoryAccess::Read;
    }
    return false;
}

bool Broadway::translateBAT(uint32_t address, MemoryAccess access,
                            uint32_t &physicalAddress) {
    bool instruction = access == MemoryAccess::Instruction;
    uint32_t first = instruction ? SPR::IBAT0U : SPR::DBAT0U;
    uint32_t second = instruction ? SPR::IBAT4U : SPR::DBAT4U;
    bool user = state.msr & 0x4000;
    for (uint32_t index = 0; index < 8; ++index) {
        uint32_t upperIndex = (index < 4 ? first : second) + (index & 3) * 2;
        uint32_t upper = state.spr[upperIndex];
        uint32_t lower = state.spr[upperIndex + 1];
        if (!(upper & (user ? 1u : 2u)))
            continue;
        uint32_t blockMask = ((upper & 0x00001FFCu) << 15) | 0x1FFFFu;
        if ((address & ~blockMask) != (upper & 0xFFFE0000u & ~blockMask))
            continue;
        uint32_t protection = lower & 3;
        if (protection == 0 ||
            (access == MemoryAccess::Write && protection != 2)) {
            if (instruction)
                raiseException(0x400, 0x08000000);
            else {
                state.spr[SPR::DAR] = address;
                state.spr[SPR::DSISR] =
                    0x08000000 |
                    (access == MemoryAccess::Write ? 0x02000000 : 0);
                raiseException(0x300);
            }
            throw MemoryAccessException{};
        }
        if (instruction && (lower & 8)) {
            raiseException(0x400, 0x10000000);
            throw MemoryAccessException{};
        }
        physicalAddress = (lower & 0xFFFE0000u & ~blockMask) | (address & blockMask);
        return true;
    }
    return false;
}

uint32_t Broadway::translatePage(uint32_t address, MemoryAccess access) {
    uint32_t segment = state.sr[address >> 28];
    bool instruction = access == MemoryAccess::Instruction;
    if (segment & 0x80000000) {
        if (instruction)
            raiseException(0x400, 0x40000000);
        else {
            state.spr[SPR::DAR] = address;
            state.spr[SPR::DSISR] =
                0x40000000 | (access == MemoryAccess::Write ? 0x02000000 : 0);
            raiseException(0x300);
        }
        throw MemoryAccessException{};
    }
    if (instruction && (segment & 0x10000000)) {
        raiseException(0x400, 0x10000000);
        throw MemoryAccessException{};
    }

    uint32_t vsid = segment & 0x00FFFFFF;
    uint32_t pageIndex = (address >> 12) & 0xFFFF;
    uint32_t api = (address >> 22) & 0x3F;
    uint32_t hash = vsid ^ pageIndex;
    uint32_t sdr1 = state.spr[SPR::SDR1];
    uint32_t tableBase = sdr1 & 0xFFFF0000;
    uint32_t tableMask = ((sdr1 & 0x1FF) << 16) | 0xFFC0;
    bool key = segment & ((state.msr & 0x4000) ? 0x20000000 : 0x40000000);

    for (uint32_t secondary = 0; secondary < 2; ++secondary) {
        uint32_t selectedHash = secondary ? ~hash : hash;
        uint32_t pteg = tableBase | ((selectedHash << 6) & tableMask);
        uint32_t expected =
            0x80000000 | (vsid << 7) | (secondary ? 0x40 : 0) | api;
        for (uint32_t entry = 0; entry < 8; ++entry) {
            uint32_t addressOfEntry = pteg + entry * 8;
            if (Bus::readPhysical32(addressOfEntry) != expected)
                continue;
            uint32_t lower = Bus::readPhysical32(addressOfEntry + 4);
            if (!protectionAllows(lower & 3, key, access)) {
                if (instruction)
                    raiseException(0x400, 0x08000000);
                else {
                    state.spr[SPR::DAR] = address;
                    state.spr[SPR::DSISR] =
                        0x08000000 |
                        (access == MemoryAccess::Write ? 0x02000000 : 0);
                    raiseException(0x300);
                }
                throw MemoryAccessException{};
            }
            if (instruction && (lower & 8)) {
                raiseException(0x400, 0x10000000);
                throw MemoryAccessException{};
            }
            uint32_t accessBits = 0x100;
            if (access == MemoryAccess::Write)
                accessBits |= 0x80;
            if ((lower & accessBits) != accessBits)
                Bus::writePhysical32(addressOfEntry + 4, lower | accessBits);
            return (lower & 0xFFFFF000) | (address & 0xFFF);
        }
    }

    if (instruction)
        raiseException(0x400, 0x40000000);
    else {
        state.spr[SPR::DAR] = address;
        state.spr[SPR::DSISR] =
            0x40000000 | (access == MemoryAccess::Write ? 0x02000000 : 0);
        raiseException(0x300);
    }
    throw MemoryAccessException{};
}

uint32_t Broadway::translateAddress(uint32_t address, MemoryAccess access) {
    bool enabled = access == MemoryAccess::Instruction ? state.msr & 0x20
                                                       : state.msr & 0x10;
    if (!enabled)
        return address;

    const uint32_t virtualPage = address & 0xFFFFF000u;
    const uint32_t tag = virtualPage | ((state.msr >> 14) & 1u);
    auto &entry = translationCache[static_cast<size_t>(access)]
                                  [(address >> 12) &
                                   (translationCacheSize - 1)];
    if (entry.valid && entry.tag == tag)
        return entry.physicalPage | (address & 0xFFFu);

    uint32_t physicalAddress;
    if (!translateBAT(address, access, physicalAddress))
        physicalAddress = translatePage(address, access);

    entry.tag = tag;
    entry.physicalPage = physicalAddress & 0xFFFFF000u;
    entry.valid = true;
    return physicalAddress;
}

void Broadway::invalidateTranslationCache() {
    for (auto &cache : translationCache) {
        for (auto &entry : cache)
            entry = {};
    }
    for (auto &entry : decodedInstructionCache)
        entry = {};
}

void Broadway::invalidateTranslationPage(uint32_t address) {
    const uint32_t virtualPage = address & 0xFFFFF000u;
    const size_t index = (address >> 12) & (translationCacheSize - 1);
    for (auto &cache : translationCache) {
        auto &entry = cache[index];
        if ((entry.tag & 0xFFFFF000u) == virtualPage)
            entry.valid = false;
    }
    for (uint32_t offset = 0; offset < 0x1000; offset += 4) {
        auto &entry = decodedInstructionCache[
            ((virtualPage + offset) >> 2) &
            (decodedInstructionCacheSize - 1)];
        if (entry.valid && (entry.tag & 0xFFFFF000u) == virtualPage)
            entry.valid = false;
    }
}

bool Broadway::branchCondition(uint32_t bo, uint32_t bi, bool useCTR) {
    if (useCTR && !(bo & 4))
        --state.spr[SPR::CTR];
    bool counter =
        !useCTR || (bo & 4) || ((state.spr[SPR::CTR] != 0) != ((bo & 2) != 0));
    bool condition =
        (bo & 16) || (((state.cr >> (31 - bi)) & 1) == ((bo >> 3) & 1));
    return counter && condition;
}

void Broadway::executeBType(uint32_t instruction) {
    uint32_t bo = (instruction >> 21) & 31;
    uint32_t bi = (instruction >> 16) & 31;
    int32_t displacement = signExtend(instruction & 0xFFFC, 16);
    if (branchCondition(bo, bi, true))
        state.nia = (instruction & 2) ? static_cast<uint32_t>(displacement)
                                      : state.cia + displacement;
    if (instruction & 1)
        state.spr[SPR::LR] = state.cia + 4;
}

void Broadway::executeSCType(uint32_t) {
    raiseException(0xC00);
    state.spr[SPR::SRR0] = state.cia + 4;
}

void Broadway::executeMType(uint32_t instruction) {
    uint32_t op = instruction >> 26;

    uint32_t rs = (instruction >> 21) & 0x1F;
    uint32_t ra = (instruction >> 16) & 0x1F;

    uint32_t sh_or_rb = (instruction >> 11) & 0x1F;

    uint32_t mb = (instruction >> 6) & 0x1F;
    uint32_t me = (instruction >> 1) & 0x1F;

    bool rc = instruction & 1;

    switch (op) {
    case static_cast<uint32_t>(BroadwayMTypeInstruction::RLWIMI): {
        uint32_t source = state.gpr[rs];
        uint32_t oldRA = state.gpr[ra];

        uint32_t rotated = std::rotl(source, sh_or_rb);

        uint32_t mask = utils::makeMask(mb, me);

        uint32_t result = (oldRA & ~mask) | (rotated & mask);

        state.gpr[ra] = result;

        if (rc)
            state.updateCR0(result);
        break;
    }
    case static_cast<uint32_t>(BroadwayMTypeInstruction::RLWINM): {
        uint32_t rotated = std::rotl(state.gpr[rs], sh_or_rb);
        uint32_t mask = utils::makeMask(mb, me);

        uint32_t result = rotated & mask;

        state.gpr[ra] = result;

        if (rc)
            state.updateCR0(result);
        break;
    }
    case static_cast<uint32_t>(BroadwayMTypeInstruction::RLWNM): {
        uint32_t amount = state.gpr[sh_or_rb] & 0x1F;

        uint32_t rotated = std::rotl(state.gpr[rs], amount);

        uint32_t result = rotated & utils::makeMask(mb, me);

        state.gpr[ra] = result;

        if (rc)
            state.updateCR0(result);
        break;
    }
    }
}

uint32_t Broadway::executeInstruction() {
    lastInstruction = 0;
    state.exceptionTaken = false;
    state.nia = state.cia + 4;
    if (deliverPendingException()) {
        state.cia = state.nia;
        return 1;
    }
    if (state.cia & 3) {
        raiseException(0x600);
        state.cia = state.nia;
        return 1;
    }
    DecodedInstruction *decoded = nullptr;
    try {
        auto &entry = decodedInstructionCache[(state.cia >> 2) &
                                              (decodedInstructionCacheSize - 1)];
        const uint32_t context = state.msr & 0x00004020u;
        if (!entry.valid || entry.tag != state.cia ||
            entry.context != context) {
            entry.tag = state.cia;
            entry.context = context;
            entry.instruction = Bus::fetch32(state.cia);
            entry.type = getInstructionType(entry.instruction);
            entry.executor = executorFor(entry.type);
            compileInstruction(entry);
            const uint32_t op = entry.instruction >> 26;
            const uint32_t xo = (entry.instruction >> 1) & 1023;
            entry.floating =
                (op >= 48 && op <= 57) || op == 59 || op == 60 || op == 61 ||
                op == 63 || (op == 4 && xo != 1014) ||
                (op == 31 &&
                 (xo == 535 || xo == 567 || xo == 599 || xo == 631 ||
                  xo == 663 || xo == 695 || xo == 727 || xo == 759 ||
                  xo == 983));
            entry.quantized = op == 4 || entry.type == InstructionType::PSQ_D;
            entry.valid = true;
        }
        decoded = &entry;
        lastInstruction = entry.instruction;
    } catch (const MemoryAccessException &) {
        if (!state.exceptionTaken) {
            Logger::log(
                "CPU", LogLevel::Error,
                "MemoryAccessException without PPC exception at CIA=0x" +
                    utils::toHexString(state.cia));

            throw;
        }
        state.cia = state.nia;
        return 1;
    }

    if (decoded->floating && !(state.msr & 0x2000)) {
        raiseException(0x800);
        state.cia = state.nia;
        return 1;
    }
    if (decoded->quantized && !(state.spr[SPR::HID2] & 0x20000000)) {
        raiseException(0x700, 0x80000);
        state.cia = state.nia;
        return 1;
    }

    try {
        executeCompiled(*decoded);
    } catch (const MemoryAccessException &) {
        if (!state.exceptionTaken) {
            Logger::log(
                "CPU", LogLevel::Error,
                "MemoryAccessException without PPC exception at CIA=0x" +
                    utils::toHexString(state.cia));

            throw;
        }
    }

    if (!state.exceptionTaken && (state.msr & 0x400)) {
        uint32_t resumeAddress = state.nia;
        raiseException(0xD00);
        state.spr[SPR::SRR0] = resumeAddress;
    }

    state.cia = state.nia;

    return 1;
}

void Broadway::setupWiiBATs() {
    state.spr[SPR::IBAT0U] = 0x80001FFF;
    state.spr[SPR::IBAT0L] = 0x00000002;

    state.spr[SPR::DBAT0U] = 0x80001FFF;
    state.spr[SPR::DBAT0L] = 0x00000002;

    state.spr[SPR::DBAT1U] = 0xC0001FFF;
    state.spr[SPR::DBAT1L] = 0x0000002A;

    state.spr[SPR::IBAT4U] = 0x90001FFF;
    state.spr[SPR::IBAT4L] = 0x10000002;

    state.spr[SPR::DBAT4U] = 0x90001FFF;
    state.spr[SPR::DBAT4L] = 0x10000002;

    state.spr[SPR::DBAT5U] = 0xD0001FFF;
    state.spr[SPR::DBAT5L] = 0x1000002A;

    invalidateTranslationCache();
}

void Broadway::advanceTime(uint64_t cycles) {
    const uint64_t partial = cycles % 12 + timeBaseRemainder;
    const uint64_t ticks = cycles / 12 + partial / 12;
    timeBaseRemainder = static_cast<uint32_t>(partial % 12);

    if (ticks == 0)
        return;

    state.timeBase += ticks;

    const uint32_t oldDec = state.spr[SPR::DEC];
    state.spr[SPR::DEC] -= static_cast<uint32_t>(ticks);

    if (ticks > oldDec)
        state.decrementerPending = true;
}

uint32_t Broadway::readSPR(uint32_t spr) {
    switch (spr) {
    case SPR::WPAR:
        return (state.spr[SPR::WPAR] & ~1u) |
               (Device::globalDevice && !Device::globalDevice->wgpipe->empty() ? 1u : 0u);
    case SPR::TBL:
        return static_cast<uint32_t>(state.timeBase & 0xFFFFFFFFull);
    case SPR::TBU:
        return static_cast<uint32_t>(state.timeBase >> 32);
    default:
        return state.spr[spr];
    }
}

void Broadway::setupWiiHLEBootState() {
    state.spr[SPR::PVR] = 0x00087102;

    state.spr[SPR::HID0] = 0x0011C664;
    state.spr[SPR::HID1] = 0x80000000;
    state.spr[SPR::HID2] = 0xE0000000;
    state.spr[SPR::HID4] = 0x83900000;
    state.spr[SPR::WPAR] = 0x0C008000;

    state.msr = 0x00002032;

    setupWiiBATs();

    state.gpr[1] = 0x816FFFF0;
}
