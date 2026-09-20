#include "cpu/broadway.h"
#include <cstdint>
#include <string>

void Broadway::reset(uint32_t entryPoint) {
    state = {};
    state.pc = entryPoint;

    instructionStream = MemoryStream(entryPoint);
}

std::string BroadwayState::log() const {
    std::string logMessage = "Broadway State:\n";
    logMessage += "PC: 0x" + std::to_string(pc) + "\n";
    logMessage += "MSR: 0x" + std::to_string(msr) + "\n";
    logMessage += "CR: 0x" + std::to_string(cr) + "\n";
    logMessage += "XER: 0x" + std::to_string(xer) + "\n";
    logMessage += "LR: 0x" + std::to_string(lr) + "\n";
    logMessage += "CTR: 0x" + std::to_string(ctr) + "\n";
    logMessage += "FPSCR: 0x" + std::to_string(fpscr) + "\n";
    logMessage += "SRR0: 0x" + std::to_string(srr0) + "\n";
    logMessage += "SRR1: 0x" + std::to_string(srr1) + "\n";
    logMessage += "DAR: 0x" + std::to_string(dar) + "\n";
    logMessage += "DSISR: 0x" + std::to_string(dsisr) + "\n";
    logMessage += "DEC: 0x" + std::to_string(dec) + "\n";
    logMessage += "Time Base: 0x" + std::to_string(timeBase) + "\n";

    logMessage += "GPRs:\n";
    for (int i = 0; i < 32; ++i) {
        logMessage += "  GPR[" + std::to_string(i) + "]: 0x" +
                      std::to_string(gpr[i]) + "\n";
    }

    logMessage += "FPRs:\n";
    for (int i = 0; i < 32; ++i) {
        logMessage += "  FPR[" + std::to_string(i) + "]: 0x" +
                      std::to_string(fpr[i]) + "\n";
    }

    return logMessage;
}

void Broadway::start() {
    while (true) {
        executeInstruction();
    }
}

void Broadway::executeInstruction() {
    uint32_t instruction = instructionStream.read32();
}