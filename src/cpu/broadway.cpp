#include "cpu/broadway.h"
#include "core/utils.h"
#include <cstdint>
#include <string>

void Broadway::reset(uint32_t entryPoint) {
    state = {};
    state.cia = entryPoint;

    instructionStream = MemoryStream(entryPoint);
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

    return InstructionType::I; // Default to I-Type for unrecognized opcodes
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

void Broadway::executeXType(uint32_t instruction) {
    uint32_t op = instruction >> 26;
    uint32_t d = (instruction >> 21) & 0x1F;
    uint32_t a = (instruction >> 16) & 0x1F;
    uint32_t b = (instruction >> 11) & 0x1F;
    uint32_t xo10 = (instruction >> 1) & 0x3FF;
    bool rc = instruction & 0x1;
}

void Broadway::executeXOType(uint32_t instruction) {
    uint32_t d = (instruction >> 21) & 0x1F;
}

void Broadway::executeInstruction() {
    uint32_t instruction = instructionStream.read32();

    state.nia = state.cia + 4;

    InstructionType type = getInstructionType(instruction);

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