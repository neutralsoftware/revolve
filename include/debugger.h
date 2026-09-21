#ifndef REVOLVE_DEBUGGER
#define REVOLVE_DEBUGGER

#include "core/executable.h"
#include "cpu/broadway.h"
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>

class Debugger {
  public:
    Debugger(Broadway &cpu, const Executable &executable);
    void run();

  private:
    struct Watchpoint {
        uint32_t address;
        uint32_t size;
        uint64_t value;
    };

    Broadway &cpu;
    const Executable &executable;
    std::set<uint32_t> breakpoints;
    std::vector<Watchpoint> watchpoints;
    std::vector<std::string> history;
    std::string previousCommand;
    bool colorsEnabled;
    bool stopOnException = true;
    bool running = true;

    void printBanner();
    void printHelp(const std::vector<std::string> &arguments);
    void printStatus();
    void printRegisters(const std::vector<std::string> &arguments);
    void printRegister(const std::vector<std::string> &arguments);
    void printMemory(const std::vector<std::string> &arguments);
    void writeMemory(const std::vector<std::string> &arguments);
    void printDisassembly(const std::vector<std::string> &arguments);
    void printInfo(const std::vector<std::string> &arguments);
    void printHistory();
    void addBreakpoint(const std::vector<std::string> &arguments);
    void removeBreakpoint(const std::vector<std::string> &arguments);
    void addWatchpoint(const std::vector<std::string> &arguments);
    void removeWatchpoint(const std::vector<std::string> &arguments);
    void step(const std::vector<std::string> &arguments);
    void next();
    void continueExecution();
    bool executeCommand(const std::string &line);
    bool executeOne(bool display);
    bool checkWatchpoints();

    std::vector<std::string> tokenize(const std::string &line) const;
    std::optional<uint64_t> parseValue(const std::string &text) const;
    std::optional<uint64_t> registerValue(const std::string &name) const;
    bool setRegisterValue(const std::string &name, uint64_t value);
    std::optional<uint64_t> inspectMemory(uint32_t address, uint32_t size);
    std::optional<uint32_t> inspectInstruction(uint32_t address);
    std::string disassemble(uint32_t instruction, uint32_t address) const;
    std::string exceptionName(uint32_t address) const;
    std::string color(const std::string &code, const std::string &text) const;
    std::string formatAddress(uint32_t value) const;
    void error(const std::string &message) const;
};

#endif
