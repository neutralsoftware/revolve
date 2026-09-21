#include "cpu/broadway.h"
#include "core/utils.h"
#include <bit>
#include <cstdint>
#include <string>

void Broadway::reset(uint32_t entryPoint) {
    state = {};
    state.cia = entryPoint;

    state.nia = entryPoint + 4;
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
    state.msr = (state.msr & ~0x0004EF36u) | ((state.msr >> 16) & 1);
    state.reservationValid = false;
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

void Broadway::executeInstruction() {
    if (state.cia & 3) {
        raiseException(0x600);
        state.cia = state.nia;
        return;
    }
    uint32_t instruction = Bus::read32(state.cia);

    state.nia = state.cia + 4;

    InstructionType type = getInstructionType(instruction);
    uint32_t op = instruction >> 26;
    uint32_t xo = (instruction >> 1) & 1023;
    bool floating = (op >= 48 && op <= 57) || op == 59 || op == 60 ||
                    op == 61 || op == 63 || (op == 4 && xo != 1014) ||
                    (op == 31 && (xo == 535 || xo == 567 || xo == 599 ||
                                  xo == 631 || xo == 663 || xo == 695 ||
                                  xo == 727 || xo == 759 || xo == 983));
    if (floating && !(state.msr & 0x2000)) {
        raiseException(0x800);
        state.cia = state.nia;
        return;
    }
    if ((op == 4 || type == InstructionType::PSQ_D) &&
        !(state.spr[SPR::HID2] & 0x20000000)) {
        raiseException(0x700, 0x80000);
        state.cia = state.nia;
        return;
    }

    switch (type) {
    case InstructionType::D:
        executeDType(instruction);
        break;
    case InstructionType::I:
        executeIType(instruction);
        break;
    case InstructionType::B:
        executeBType(instruction);
        break;
    case InstructionType::SC:
        executeSCType(instruction);
        break;
    case InstructionType::X:
        executeXType(instruction);
        break;
    case InstructionType::XO:
        executeXOType(instruction);
        break;
    case InstructionType::XFX:
        executeXFXType(instruction);
        break;
    case InstructionType::XFL:
        executeXFLType(instruction);
        break;
    case InstructionType::XL:
        executeXLType(instruction);
        break;
    case InstructionType::M:
        executeMType(instruction);
        break;
    case InstructionType::A:
        executeAType(instruction);
        break;
    case InstructionType::PSQ_D:
        executePSQ_DType(instruction);
        break;
    case InstructionType::PSQ_X:
        executePSQ_XType(instruction);
        break;
    default:
        Logger::log("Broadway", LogLevel::Error,
                    "executeInstruction: Unknown instruction type");
        break;
    }

    state.cia = state.nia;
}
