#include "debugger.h"
#include "SDL3/SDL_events.h"
#include "core/memory.h"
#include "core/utils.h"
#include "device.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <tuple>
#include <unistd.h>
#include <unordered_map>

namespace {
std::atomic_bool interrupted = false;

void interruptHandler(int) { interrupted = true; }

std::string lower(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char character) { return std::tolower(character); });
    return value;
}

std::string hexValue(uint64_t value, uint32_t width) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setfill('0')
           << std::setw(width) << value;
    return stream.str();
}
} // namespace

Debugger::Debugger(Broadway &cpu, const Executable &executable)
    : cpu(cpu), executable(executable),
      colorsEnabled(isatty(STDOUT_FILENO) &&
                    std::getenv("NO_COLOR") == nullptr) {}

std::string Debugger::color(const std::string &code,
                            const std::string &text) const {
    return colorsEnabled ? "\033[" + code + "m" + text + "\033[0m" : text;
}

std::string Debugger::formatAddress(uint32_t value) const {
    return color("36", hexValue(value, 8));
}

void Debugger::error(const std::string &message) const {
    std::cout << color("1;31", "error") << "  " << message << '\n';
}

void Debugger::printBanner() {
    std::cout << color("1;35",
                       "╭──────────────────────────────────────────────╮")
              << '\n'
              << color("1;35", "│")
              << color("1;37", "  REVOLVE // BROADWAY DEBUG CONSOLE           ")
              << color("1;35", "│") << '\n'
              << color("1;35",
                       "╰──────────────────────────────────────────────╯")
              << '\n'
              << color("90", "entry  ") << formatAddress(executable.entryPoint)
              << color("90", "   type ") << color("1;33", "help")
              << color("90", " for commands") << "\n\n";
    printStatus();
}

void Debugger::run() {
    std::signal(SIGINT, interruptHandler);
    printBanner();
    while (running) {
        std::cout << '\n'
                  << color("1;35", "revolve") << color("90", ":")
                  << formatAddress(cpu.state.cia) << color("1;35", " ❯ ")
                  << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }
        if (line.empty()) {
            if (previousCommand.empty())
                continue;
            line = previousCommand;
        } else {
            previousCommand = line;
            history.push_back(line);
        }
        executeCommand(line);
    }
    std::signal(SIGINT, SIG_DFL);
}

std::vector<std::string> Debugger::tokenize(const std::string &line) const {
    std::vector<std::string> tokens;
    std::string token;
    bool quoted = false;
    char quote = 0;
    for (char character : line) {
        if (quoted) {
            if (character == quote) {
                quoted = false;
            } else {
                token += character;
            }
        } else if (character == '\'' || character == '"') {
            quoted = true;
            quote = character;
        } else if (std::isspace(static_cast<unsigned char>(character))) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += character;
        }
    }
    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

std::optional<uint64_t> Debugger::registerValue(const std::string &name) const {
    std::string key = lower(name);
    if (!key.empty() && key[0] == '$')
        key.erase(0, 1);
    auto index = [&](std::string_view prefix,
                     uint32_t count) -> std::optional<uint32_t> {
        if (!key.starts_with(prefix))
            return std::nullopt;
        std::string suffix = key.substr(prefix.size());
        if (suffix.empty() ||
            !std::all_of(suffix.begin(), suffix.end(), ::isdigit))
            return std::nullopt;
        uint32_t value = static_cast<uint32_t>(std::stoul(suffix));
        return value < count ? std::optional<uint32_t>(value) : std::nullopt;
    };
    if (auto value = index("r", 32))
        return cpu.state.gpr[*value];
    if (auto value = index("f", 32))
        return cpu.state.fpr[*value];
    if (auto value = index("sr", 16))
        return cpu.state.sr[*value];
    if (key == "pc" || key == "cia")
        return cpu.state.cia;
    if (key == "nia")
        return cpu.state.nia;
    if (key == "lr")
        return cpu.state.spr[SPR::LR];
    if (key == "ctr")
        return cpu.state.spr[SPR::CTR];
    if (key == "xer")
        return cpu.state.spr[SPR::XER];
    if (key == "cr")
        return cpu.state.cr;
    if (key == "msr")
        return cpu.state.msr;
    if (key == "fpscr")
        return cpu.state.fpscr;
    if (key == "dar")
        return cpu.state.spr[SPR::DAR];
    if (key == "dsisr")
        return cpu.state.spr[SPR::DSISR];
    if (key == "srr0")
        return cpu.state.spr[SPR::SRR0];
    if (key == "srr1")
        return cpu.state.spr[SPR::SRR1];
    if (key == "dec")
        return cpu.state.spr[SPR::DEC];
    if (key == "sdr1")
        return cpu.state.spr[SPR::SDR1];
    if (key == "tb")
        return cpu.state.timeBase;
    return std::nullopt;
}

std::optional<uint64_t> Debugger::parseValue(const std::string &text) const {
    if (auto value = registerValue(text))
        return value;
    try {
        size_t consumed = 0;
        uint64_t value = std::stoull(text, &consumed, 0);
        if (consumed == text.size())
            return value;
    } catch (...) {
    }
    return std::nullopt;
}

bool Debugger::setRegisterValue(const std::string &name, uint64_t value) {
    std::string key = lower(name);
    if (!key.empty() && key[0] == '$')
        key.erase(0, 1);
    auto index = [&](std::string_view prefix,
                     uint32_t count) -> std::optional<uint32_t> {
        if (!key.starts_with(prefix))
            return std::nullopt;
        std::string suffix = key.substr(prefix.size());
        if (suffix.empty() ||
            !std::all_of(suffix.begin(), suffix.end(), ::isdigit))
            return std::nullopt;
        uint32_t parsed = static_cast<uint32_t>(std::stoul(suffix));
        return parsed < count ? std::optional<uint32_t>(parsed) : std::nullopt;
    };
    if (auto parsed = index("r", 32))
        cpu.state.gpr[*parsed] = static_cast<uint32_t>(value);
    else if (auto parsed = index("f", 32))
        cpu.state.fpr[*parsed] = value;
    else if (auto parsed = index("sr", 16))
        cpu.state.sr[*parsed] = static_cast<uint32_t>(value);
    else if (key == "pc" || key == "cia")
        cpu.state.cia = static_cast<uint32_t>(value);
    else if (key == "nia")
        cpu.state.nia = static_cast<uint32_t>(value);
    else if (key == "lr")
        cpu.state.spr[SPR::LR] = static_cast<uint32_t>(value);
    else if (key == "ctr")
        cpu.state.spr[SPR::CTR] = static_cast<uint32_t>(value);
    else if (key == "xer")
        cpu.state.spr[SPR::XER] = static_cast<uint32_t>(value);
    else if (key == "cr")
        cpu.state.cr = static_cast<uint32_t>(value);
    else if (key == "msr")
        cpu.state.msr = static_cast<uint32_t>(value);
    else if (key == "fpscr")
        cpu.state.fpscr = static_cast<uint32_t>(value);
    else if (key == "dar")
        cpu.state.spr[SPR::DAR] = static_cast<uint32_t>(value);
    else if (key == "dsisr")
        cpu.state.spr[SPR::DSISR] = static_cast<uint32_t>(value);
    else if (key == "srr0")
        cpu.state.spr[SPR::SRR0] = static_cast<uint32_t>(value);
    else if (key == "srr1")
        cpu.state.spr[SPR::SRR1] = static_cast<uint32_t>(value);
    else if (key == "dec")
        cpu.state.spr[SPR::DEC] = static_cast<uint32_t>(value);
    else if (key == "sdr1")
        cpu.state.spr[SPR::SDR1] = static_cast<uint32_t>(value);
    else if (key == "tb")
        cpu.state.timeBase = value;
    else
        return false;
    return true;
}

std::optional<uint64_t> Debugger::inspectMemory(uint32_t address,
                                                uint32_t size) {
    BroadwayState saved = cpu.state;
    try {
        uint64_t value = 0;
        if (size == 1)
            value = Bus::read8(address);
        else if (size == 2)
            value = Bus::read16(address);
        else if (size == 4)
            value = Bus::read32(address);
        else if (size == 8)
            value = Bus::read64(address);
        else {
            cpu.state = saved;
            return std::nullopt;
        }
        cpu.state = saved;
        return value;
    } catch (const MemoryAccessException &) {
        cpu.state = saved;
        return std::nullopt;
    }
}

std::optional<uint32_t> Debugger::inspectInstruction(uint32_t address) {
    BroadwayState saved = cpu.state;
    try {
        uint32_t instruction = Bus::fetch32(address);
        cpu.state = saved;
        return instruction;
    } catch (const MemoryAccessException &) {
        cpu.state = saved;
        return std::nullopt;
    }
}

std::string Debugger::disassemble(uint32_t instruction,
                                  uint32_t address) const {
    static const std::unordered_map<uint32_t, std::string> primary = {
        {3, "twi"},     {7, "mulli"},   {8, "subfic"},  {10, "cmpli"},
        {11, "cmpi"},   {12, "addic"},  {13, "addic."}, {14, "addi"},
        {15, "addis"},  {20, "rlwimi"}, {21, "rlwinm"}, {23, "rlwnm"},
        {24, "ori"},    {25, "oris"},   {26, "xori"},   {27, "xoris"},
        {28, "andi."},  {29, "andis."}, {32, "lwz"},    {33, "lwzu"},
        {34, "lbz"},    {35, "lbzu"},   {36, "stw"},    {37, "stwu"},
        {38, "stb"},    {39, "stbu"},   {40, "lhz"},    {41, "lhzu"},
        {42, "lha"},    {43, "lhau"},   {44, "sth"},    {45, "sthu"},
        {46, "lmw"},    {47, "stmw"},   {48, "lfs"},    {49, "lfsu"},
        {50, "lfd"},    {51, "lfdu"},   {52, "stfs"},   {53, "stfsu"},
        {54, "stfd"},   {55, "stfdu"},  {56, "psq_l"},  {57, "psq_lu"},
        {60, "psq_st"}, {61, "psq_stu"}};
    static const std::unordered_map<uint32_t, std::string> op31 = {
        {0, "cmp"},      {4, "tw"},       {8, "subfc"},    {10, "addc"},
        {11, "mulhwu"},  {19, "mfcr"},    {20, "lwarx"},   {23, "lwzx"},
        {24, "slw"},     {26, "cntlzw"},  {28, "and"},     {32, "cmpl"},
        {40, "subf"},    {54, "dcbst"},   {55, "lwzux"},   {60, "andc"},
        {75, "mulhw"},   {83, "mfmsr"},   {86, "dcbf"},    {87, "lbzx"},
        {104, "neg"},    {119, "lbzux"},  {124, "nor"},    {136, "subfe"},
        {138, "adde"},   {144, "mtcrf"},  {146, "mtmsr"},  {150, "stwcx."},
        {151, "stwx"},   {183, "stwux"},  {200, "subfze"}, {202, "addze"},
        {210, "mtsr"},   {215, "stbx"},   {232, "subfme"}, {234, "addme"},
        {235, "mullw"},  {242, "mtsrin"}, {246, "dcbtst"}, {247, "stbux"},
        {266, "add"},    {278, "dcbt"},   {279, "lhzx"},   {284, "eqv"},
        {306, "tlbie"},  {310, "eciwx"},  {311, "lhzux"},  {316, "xor"},
        {339, "mfspr"},  {343, "lhax"},   {371, "mftb"},   {375, "lhaux"},
        {407, "sthx"},   {412, "orc"},    {438, "ecowx"},  {439, "sthux"},
        {444, "or"},     {459, "divwu"},  {467, "mtspr"},  {470, "dcbi"},
        {476, "nand"},   {491, "divw"},   {512, "mcrxr"},  {533, "lswx"},
        {534, "lwbrx"},  {535, "lfsx"},   {536, "srw"},    {566, "tlbsync"},
        {567, "lfsux"},  {595, "mfsr"},   {597, "lswi"},   {598, "sync"},
        {599, "lfdx"},   {631, "lfdux"},  {659, "mfsrin"}, {661, "stswx"},
        {662, "stwbrx"}, {663, "stfsx"},  {695, "stfsux"}, {725, "stswi"},
        {727, "stfdx"},  {758, "dcba"},   {759, "stfdux"}, {790, "lhbrx"},
        {792, "sraw"},   {824, "srawi"},  {854, "eieio"},  {918, "sthbrx"},
        {922, "extsh"},  {954, "extsb"},  {982, "icbi"},   {983, "stfiwx"},
        {1014, "dcbz"}};
    static const std::unordered_map<uint32_t, std::string> op19 = {
        {0, "mcrf"},     {16, "bclr"},   {33, "crnor"},  {50, "rfi"},
        {129, "crandc"}, {150, "isync"}, {193, "crxor"}, {225, "crnand"},
        {257, "crand"},  {289, "creqv"}, {417, "crorc"}, {449, "cror"},
        {528, "bcctr"}};
    static const std::unordered_map<uint32_t, std::string> floating = {
        {12, "frsp"},   {14, "fctiw"},   {15, "fctiwz"}, {18, "fdiv"},
        {20, "fsub"},   {21, "fadd"},    {23, "fsel"},   {24, "fres"},
        {25, "fmul"},   {26, "frsqrte"}, {28, "fmsub"},  {29, "fmadd"},
        {30, "fnmsub"}, {31, "fnmadd"},  {32, "fcmpo"},  {40, "fneg"},
        {64, "mcrfs"},  {70, "mtfsb0"},  {72, "fmr"},    {134, "mtfsfi"},
        {136, "fnabs"}, {264, "fabs"},   {583, "mffs"},  {711, "mtfsf"}};
    static const std::unordered_map<uint32_t, std::string> paired = {
        {0, "ps_cmpu0"},     {6, "psq_lx"},       {7, "psq_stx"},
        {10, "ps_sum0"},     {11, "ps_sum1"},     {12, "ps_muls0"},
        {13, "ps_muls1"},    {14, "ps_madds0"},   {15, "ps_madds1"},
        {18, "ps_div"},      {20, "ps_sub"},      {21, "ps_add"},
        {23, "ps_sel"},      {24, "ps_res"},      {25, "ps_mul"},
        {26, "ps_rsqrte"},   {28, "ps_msub"},     {29, "ps_madd"},
        {30, "ps_nmsub"},    {31, "ps_nmadd"},    {32, "ps_cmpo0"},
        {38, "psq_lux"},     {39, "psq_stux"},    {40, "ps_neg"},
        {64, "ps_cmpu1"},    {72, "ps_mr"},       {96, "ps_cmpo1"},
        {136, "ps_nabs"},    {264, "ps_abs"},     {528, "ps_merge00"},
        {560, "ps_merge01"}, {592, "ps_merge10"}, {624, "ps_merge11"},
        {1014, "dcbz_l"}};

    uint32_t op = instruction >> 26;
    uint32_t d = (instruction >> 21) & 31;
    uint32_t a = (instruction >> 16) & 31;
    uint32_t b = (instruction >> 11) & 31;
    uint32_t c = (instruction >> 6) & 31;
    uint32_t xo = (instruction >> 1) & 1023;
    bool rc = instruction & 1;
    std::string name = ".long";
    std::string operands = hexValue(instruction, 8);

    if (op == 18) {
        int32_t displacement = signExtend(instruction & 0x03FFFFFC, 26);
        uint32_t target =
            instruction & 2 ? displacement : address + displacement;
        name = instruction & 1 ? "bl" : "b";
        operands = hexValue(target, 8);
    } else if (op == 16) {
        int32_t displacement = signExtend(instruction & 0xFFFC, 16);
        uint32_t target =
            instruction & 2 ? displacement : address + displacement;
        name = instruction & 1 ? "bcl" : "bc";
        operands = std::to_string(d) + ", " + std::to_string(a) + ", " +
                   hexValue(target, 8);
    } else if (op == 17) {
        name = "sc";
        operands.clear();
    } else if (op == 31) {
        uint32_t lookup = xo;
        bool overflow = false;
        if (!op31.contains(lookup) && op31.contains(lookup & 511)) {
            lookup &= 511;
            overflow = true;
        }
        if (auto found = op31.find(lookup); found != op31.end()) {
            name = found->second + (overflow ? "o" : "") +
                   (rc && found->second.back() != '.' ? "." : "");
            operands = "r" + std::to_string(d) + ", r" + std::to_string(a) +
                       ", r" + std::to_string(b);
            if (lookup == 339 || lookup == 467 || lookup == 371) {
                uint32_t spr =
                    ((instruction >> 16) & 31) | ((instruction >> 6) & 992);
                operands = "r" + std::to_string(d) + ", " + std::to_string(spr);
            }
        }
    } else if (op == 19) {
        if (auto found = op19.find(xo); found != op19.end()) {
            name = found->second + (rc && (xo == 16 || xo == 528) ? "l" : "");
            operands = std::to_string(d) + ", " + std::to_string(a) + ", " +
                       std::to_string(b);
        }
    } else if (op == 59 || op == 63) {
        uint32_t lookup = xo;
        if (op == 59)
            lookup &= 31;
        else if (!floating.contains(lookup) && floating.contains(lookup & 31))
            lookup &= 31;
        if (auto found = floating.find(lookup); found != floating.end()) {
            name = found->second;
            if (op == 59 && name != "fres")
                name += "s";
            if (rc)
                name += ".";
            operands = "f" + std::to_string(d) + ", f" + std::to_string(a) +
                       ", f" + std::to_string(b) + ", f" + std::to_string(c);
        }
    } else if (op == 4) {
        uint32_t lookup = xo;
        if (!paired.contains(lookup) && paired.contains(lookup & 31))
            lookup &= 31;
        if (auto found = paired.find(lookup); found != paired.end()) {
            name = found->second + (rc ? "." : "");
            operands = "f" + std::to_string(d) + ", f" + std::to_string(a) +
                       ", f" + std::to_string(b) + ", f" + std::to_string(c);
        }
    } else if (auto found = primary.find(op); found != primary.end()) {
        name = found->second;
        int32_t immediate = signExtend(instruction & 0xFFFF, 16);
        if (op >= 32 && op <= 55) {
            std::string reg = op >= 48 ? "f" : "r";
            operands = reg + std::to_string(d) + ", " +
                       std::to_string(immediate) + "(r" + std::to_string(a) +
                       ")";
        } else if (op >= 24 && op <= 29) {
            operands = "r" + std::to_string(a) + ", r" + std::to_string(d) +
                       ", " + hexValue(instruction & 0xFFFF, 4);
        } else {
            operands = "r" + std::to_string(d) + ", r" + std::to_string(a) +
                       ", " + std::to_string(immediate);
        }
    }

    return color("1;33", name) +
           (operands.empty() ? "" : " " + color("37", operands));
}

std::string Debugger::exceptionName(uint32_t address) const {
    uint32_t vector = address & 0xFFFFF;
    static const std::unordered_map<uint32_t, std::string> names = {
        {0x100, "System Reset"},
        {0x200, "Machine Check"},
        {0x300, "Data Storage"},
        {0x400, "Instruction Storage"},
        {0x500, "External Interrupt"},
        {0x600, "Alignment"},
        {0x700, "Program"},
        {0x800, "FPU Unavailable"},
        {0x900, "Decrementer"},
        {0xC00, "System Call"},
        {0xD00, "Trace"},
        {0xF00, "Performance Monitor"},
        {0x1300, "Instruction Address Breakpoint"},
        {0x1700, "Thermal Management"}};
    if (auto found = names.find(vector); found != names.end())
        return found->second;
    return "Exception";
}

void Debugger::printStatus() {
    auto instruction = inspectInstruction(cpu.state.cia);
    std::cout << color("90", "pc     ") << formatAddress(cpu.state.cia)
              << color("90", "   msr ") << formatAddress(cpu.state.msr)
              << color("90", "   lr ") << formatAddress(cpu.state.spr[SPR::LR])
              << color("90", "   ctr ")
              << formatAddress(cpu.state.spr[SPR::CTR]) << '\n';
    if (instruction)
        std::cout << color("1;35", "→") << "  " << formatAddress(cpu.state.cia)
                  << "  " << color("90", hexValue(*instruction, 8)) << "  "
                  << disassemble(*instruction, cpu.state.cia) << '\n';
    else
        std::cout << color("1;31", "×") << "  " << formatAddress(cpu.state.cia)
                  << color("31", "  instruction is not readable") << '\n';
}

void Debugger::printHelp(const std::vector<std::string> &arguments) {
    if (arguments.size() > 1) {
        std::string command = lower(arguments[1]);
        static const std::unordered_map<std::string, std::string> details = {
            {"step",
             "step [count]                 Execute one or more instructions"},
            {"next", "next                         Step over a linked branch"},
            {"frame", "frame                        Skip to the next frame"},
            {"continue",
             "continue                     Run until a stop condition"},
            {"break",
             "break <address|register>      Add an execution breakpoint"},
            {"delete",
             "delete <address|all>          Remove execution breakpoints"},
            {"watch", "watch <address> [1|2|4|8]     Stop when memory changes"},
            {"x", "x[/count][x|d|u|c][b|h|w|g] <address>"},
            {"write", "write <address> <value> [8|16|32|64]"},
            {"reg", "reg <name> [value]           Read or change one register"},
            {"regs", "regs [gpr|fpr|spr|all]       Display register groups"},
            {"disasm", "disasm [address] [count]     Disassemble memory"},
            {"info", "info [break|watch|mmu|sections|exceptions]"}};
        if (auto found = details.find(command); found != details.end())
            std::cout << color("1;33", found->second) << '\n';
        else
            error("no detailed help for '" + command + "'");
        return;
    }
    std::cout << color("1;37", "Execution") << '\n'
              << "  " << color("1;33", "s, step") << color("90", " [count]")
              << "       step instructions\n"
              << "  " << color("1;33", "n, next")
              << "               step over a linked branch\n"
              << "  " << color("1;33", "c, continue")
              << "           continue execution\n"
              << "  " << color("1;33", "b, break") << color("90", " <address>")
              << "     add a breakpoint\n"
              << "  " << color("1;33", "d, delete")
              << color("90", " <address|all>") << " remove breakpoints\n"
              << "  " << color("1;33", "watch")
              << color("90", " <address> [size]")
              << " watch memory for changes\n"
              << "  " << color("1;33", "unwatch")
              << color("90", " <address|all>") << "  remove watchpoints\n\n"
              << color("1;37", "Inspection") << '\n'
              << "  " << color("1;33", "status")
              << "                show the current instruction\n"
              << "  " << color("1;33", "regs")
              << color("90", " [gpr|fpr|spr|all]") << " display registers\n"
              << "  " << color("1;33", "reg") << color("90", " <name> [value]")
              << "    inspect or edit a register\n"
              << "  " << color("1;33", "x/16xb") << color("90", " <address>")
              << "      inspect memory\n"
              << "  " << color("1;33", "write")
              << color("90", " <address> <value> [bits]") << " write memory\n"
              << "  " << color("1;33", "disasm")
              << color("90", " [address] [count]")
              << " disassemble instructions\n"
              << "  " << color("1;33", "info")
              << color("90", " [break|watch|mmu|sections|exceptions]") << "\n\n"
              << color("1;37", "Console") << '\n'
              << "  " << color("1;33", "set stop-exceptions")
              << color("90", " <on|off>") << " configure exception stops\n"
              << "  " << color("1;33", "history")
              << "               show command history\n"
              << "  " << color("1;33", "clear")
              << "                 clear the terminal\n"
              << "  " << color("1;33", "q, quit")
              << "               leave the debugger\n"
              << color("90", "Empty input repeats the previous command. Values "
                             "accept decimal, 0x hex, or register names.")
              << '\n';
}

void Debugger::printRegisters(const std::vector<std::string> &arguments) {
    std::string group = arguments.size() > 1 ? lower(arguments[1]) : "gpr";
    if (group == "gpr" || group == "all") {
        std::cout << color("1;37", "General-purpose registers") << '\n';
        for (uint32_t row = 0; row < 8; ++row) {
            for (uint32_t column = 0; column < 4; ++column) {
                uint32_t index = row + column * 8;
                std::ostringstream name;
                name << 'r' << std::left << std::setw(2) << index;
                std::cout << color("1;33", name.str()) << ' '
                          << formatAddress(cpu.state.gpr[index])
                          << (column == 3 ? '\n' : ' ');
            }
        }
    }
    if (group == "fpr" || group == "all") {
        std::cout << color("1;37", "Floating-point registers") << '\n';
        for (uint32_t index = 0; index < 32; ++index) {
            double value = std::bit_cast<double>(cpu.state.fpr[index]);
            std::ostringstream rendered;
            rendered << std::setprecision(12) << value;
            std::ostringstream name;
            name << 'f' << std::left << std::setw(2) << index;
            std::cout << color("1;33", name.str()) << ' '
                      << color("36", hexValue(cpu.state.fpr[index], 16)) << "  "
                      << color("37", rendered.str()) << '\n';
        }
    }
    if (group == "spr" || group == "all") {
        std::cout << color("1;37", "Control registers") << '\n';
        std::array<std::pair<std::string, uint64_t>, 14> values = {
            {{"pc", cpu.state.cia},
             {"nia", cpu.state.nia},
             {"lr", cpu.state.spr[SPR::LR]},
             {"ctr", cpu.state.spr[SPR::CTR]},
             {"cr", cpu.state.cr},
             {"xer", cpu.state.spr[SPR::XER]},
             {"msr", cpu.state.msr},
             {"fpscr", cpu.state.fpscr},
             {"dar", cpu.state.spr[SPR::DAR]},
             {"dsisr", cpu.state.spr[SPR::DSISR]},
             {"srr0", cpu.state.spr[SPR::SRR0]},
             {"srr1", cpu.state.spr[SPR::SRR1]},
             {"dec", cpu.state.spr[SPR::DEC]},
             {"tb", cpu.state.timeBase}}};
        for (uint32_t index = 0; index < values.size(); ++index) {
            std::cout << color("1;33", values[index].first) << ' '
                      << color("36",
                               hexValue(values[index].second,
                                        values[index].first == "tb" ? 16 : 8))
                      << (index % 2 ? "\n" : "    ");
        }
    }
    if (group != "gpr" && group != "fpr" && group != "spr" && group != "all")
        error("unknown register group '" + group + "'");
}

void Debugger::printRegister(const std::vector<std::string> &arguments) {
    if (arguments.size() < 2) {
        error("usage: reg <name> [value]");
        return;
    }
    if (arguments.size() > 2) {
        auto value = parseValue(arguments[2]);
        if (!value) {
            error("invalid value '" + arguments[2] + "'");
            return;
        }
        if (!setRegisterValue(arguments[1], *value)) {
            error("unknown register '" + arguments[1] + "'");
            return;
        }
    }
    auto value = registerValue(arguments[1]);
    if (!value) {
        error("unknown register '" + arguments[1] + "'");
        return;
    }
    std::cout << color("1;33", lower(arguments[1])) << " = "
              << color("36", hexValue(*value, *value > 0xFFFFFFFF ? 16 : 8))
              << color("90", "  (") << color("37", std::to_string(*value))
              << color("90", ")") << '\n';
}

void Debugger::printMemory(const std::vector<std::string> &arguments) {
    if (arguments.size() < 2) {
        error("usage: x[/count][x|d|u|c][b|h|w|g] <address>");
        return;
    }

    uint32_t count = 16;
    uint32_t size = 1;
    char format = 'x';
    std::string specification = lower(arguments[0]);
    if (specification.size() > 1) {
        if (specification[1] != '/') {
            error("invalid memory format");
            return;
        }
        std::string options = specification.substr(2);
        size_t position = 0;
        while (position < options.size() &&
               std::isdigit(static_cast<unsigned char>(options[position])))
            ++position;
        if (position != 0) {
            count =
                static_cast<uint32_t>(std::stoul(options.substr(0, position)));
            if (count == 0 || count > 4096) {
                error("memory count must be between 1 and 4096");
                return;
            }
        }
        for (; position < options.size(); ++position) {
            switch (options[position]) {
            case 'x':
            case 'd':
            case 'u':
            case 'c':
                format = options[position];
                break;
            case 'b':
                size = 1;
                break;
            case 'h':
                size = 2;
                break;
            case 'w':
                size = 4;
                break;
            case 'g':
                size = 8;
                break;
            default:
                error("invalid memory format character");
                return;
            }
        }
    }

    auto parsedAddress = parseValue(arguments[1]);
    if (!parsedAddress) {
        error("invalid address '" + arguments[1] + "'");
        return;
    }
    uint32_t address = static_cast<uint32_t>(*parsedAddress);

    if (size == 1 && format == 'x') {
        for (uint32_t offset = 0; offset < count; offset += 16) {
            std::cout << formatAddress(address + offset) << color("90", "  ");
            std::string ascii;
            for (uint32_t column = 0; column < 16; ++column) {
                if (offset + column < count) {
                    auto value = inspectMemory(address + offset + column, 1);
                    if (value) {
                        std::cout << color("36", hexValue(*value, 2).substr(2));
                        unsigned char character =
                            static_cast<unsigned char>(*value);
                        ascii += std::isprint(character) ? character : '.';
                    } else {
                        std::cout << color("31", "??");
                        ascii += '?';
                    }
                    std::cout << ' ';
                } else {
                    std::cout << "   ";
                    ascii += ' ';
                }
                if (column == 7)
                    std::cout << ' ';
            }
            std::cout << color("90", " │") << color("37", ascii)
                      << color("90", "│") << '\n';
        }
        return;
    }

    for (uint32_t index = 0; index < count; ++index) {
        uint32_t current = address + index * size;
        auto value = inspectMemory(current, size);
        std::cout << formatAddress(current) << color("90", "  ");
        if (!value) {
            std::cout << color("31", "unreadable") << '\n';
            continue;
        }
        if (format == 'x')
            std::cout << color("36", hexValue(*value, size * 2));
        else if (format == 'd') {
            int64_t signedValue = static_cast<int64_t>(*value);
            if (size < 8) {
                uint64_t sign = uint64_t{1} << (size * 8 - 1);
                signedValue = static_cast<int64_t>((*value ^ sign) - sign);
            }
            std::cout << color("37", std::to_string(signedValue));
        } else if (format == 'u')
            std::cout << color("37", std::to_string(*value));
        else {
            unsigned char character = static_cast<unsigned char>(*value);
            std::cout << color("37", std::isprint(character)
                                         ? std::string(1, character)
                                         : ".");
        }
        std::cout << '\n';
    }
}

void Debugger::writeMemory(const std::vector<std::string> &arguments) {
    if (arguments.size() < 3) {
        error("usage: write <address> <value> [8|16|32|64]");
        return;
    }
    auto address = parseValue(arguments[1]);
    auto value = parseValue(arguments[2]);
    auto bits = arguments.size() > 3 ? parseValue(arguments[3])
                                     : std::optional<uint64_t>(32);
    if (!address || !value || !bits ||
        (*bits != 8 && *bits != 16 && *bits != 32 && *bits != 64)) {
        error("invalid address, value, or width");
        return;
    }

    BroadwayState saved = cpu.state;
    try {
        if (*bits == 8)
            Bus::write8(static_cast<uint32_t>(*address),
                        static_cast<uint8_t>(*value));
        else if (*bits == 16)
            Bus::write16(static_cast<uint32_t>(*address),
                         static_cast<uint16_t>(*value));
        else if (*bits == 32)
            Bus::write32(static_cast<uint32_t>(*address),
                         static_cast<uint32_t>(*value));
        else
            Bus::write64(static_cast<uint32_t>(*address), *value);
        cpu.state = saved;
        std::cout << color("1;32", "written") << "  "
                  << formatAddress(static_cast<uint32_t>(*address)) << " = "
                  << color("36",
                           hexValue(*value, static_cast<uint32_t>(*bits / 4)))
                  << '\n';
    } catch (const MemoryAccessException &) {
        cpu.state = saved;
        error("memory write failed");
    }
}

void Debugger::printDisassembly(const std::vector<std::string> &arguments) {
    uint32_t address = cpu.state.cia;
    uint32_t count = 10;
    if (arguments.size() > 1) {
        auto parsed = parseValue(arguments[1]);
        if (!parsed) {
            error("invalid disassembly address");
            return;
        }
        address = static_cast<uint32_t>(*parsed);
    }
    if (arguments.size() > 2) {
        auto parsed = parseValue(arguments[2]);
        if (!parsed || *parsed == 0 || *parsed > 256) {
            error("disassembly count must be between 1 and 256");
            return;
        }
        count = static_cast<uint32_t>(*parsed);
    }
    for (uint32_t index = 0; index < count; ++index) {
        uint32_t current = address + index * 4;
        auto instruction = inspectInstruction(current);
        std::string marker = current == cpu.state.cia ? color("1;35", "→")
                             : breakpoints.contains(current)
                                 ? color("1;31", "●")
                                 : " ";
        std::cout << marker << "  " << formatAddress(current) << "  ";
        if (instruction)
            std::cout << color("90", hexValue(*instruction, 8)) << "  "
                      << disassemble(*instruction, current);
        else
            std::cout << color("31", "????????  unreadable");
        std::cout << '\n';
    }
}

void Debugger::printInfo(const std::vector<std::string> &arguments) {
    std::string topic = arguments.size() > 1 ? lower(arguments[1]) : "break";
    if (topic == "break" || topic == "breakpoints") {
        if (breakpoints.empty()) {
            std::cout << color("90", "No breakpoints.") << '\n';
            return;
        }
        for (uint32_t address : breakpoints)
            std::cout << color("1;31", "●") << "  " << formatAddress(address)
                      << '\n';
    } else if (topic == "watch" || topic == "watchpoints") {
        if (watchpoints.empty()) {
            std::cout << color("90", "No watchpoints.") << '\n';
            return;
        }
        for (const auto &watchpoint : watchpoints)
            std::cout << color("1;33", "◆") << "  "
                      << formatAddress(watchpoint.address)
                      << color("90", "  size ") << watchpoint.size
                      << color("90", "  value ")
                      << color("36",
                               hexValue(watchpoint.value, watchpoint.size * 2))
                      << '\n';
    } else if (topic == "mmu") {
        std::cout << color("1;37", "MMU") << color("90", "  instruction ")
                  << color(cpu.state.msr & 0x20 ? "1;32" : "1;31",
                           cpu.state.msr & 0x20 ? "on" : "off")
                  << color("90", "  data ")
                  << color(cpu.state.msr & 0x10 ? "1;32" : "1;31",
                           cpu.state.msr & 0x10 ? "on" : "off")
                  << color("90", "  sdr1 ")
                  << formatAddress(cpu.state.spr[SPR::SDR1]) << '\n';

        for (uint32_t index = 0; index < 16; ++index) {
            std::cout << color("1;33", "sr" + std::to_string(index)) << ' '
                      << formatAddress(cpu.state.sr[index])
                      << (index % 4 == 3 ? '\n' : ' ');
        }

        std::cout << '\n';

        for (uint32_t index = 0; index < 8; ++index) {
            uint32_t ibat =
                index < 4
                    ? static_cast<uint32_t>(SPR::IBAT0U) + index * 2
                    : static_cast<uint32_t>(SPR::IBAT4U) + (index - 4) * 2;

            uint32_t dbat =
                index < 4
                    ? static_cast<uint32_t>(SPR::DBAT0U) + index * 2
                    : static_cast<uint32_t>(SPR::DBAT4U) + (index - 4) * 2;

            std::cout << color("1;33", "ibat" + std::to_string(index))
                      << color("90", " U=")
                      << formatAddress(cpu.state.spr[ibat])
                      << color("90", " L=")
                      << formatAddress(cpu.state.spr[ibat + 1]) << '\n';

            std::cout << color("1;33", "dbat" + std::to_string(index))
                      << color("90", " U=")
                      << formatAddress(cpu.state.spr[dbat])
                      << color("90", " L=")
                      << formatAddress(cpu.state.spr[dbat + 1]) << '\n';
        }
    } else if (topic == "sections") {
        for (uint32_t index = 0; index < executable.textSections.size();
             ++index) {
            const auto &section = executable.textSections[index];
            if (section.size)
                std::cout << color("1;35", "text" + std::to_string(index))
                          << "  " << formatAddress(section.loadAddress) << " - "
                          << formatAddress(section.loadAddress + section.size)
                          << color("90", "  ") << section.size << " bytes\n";
        }
        for (uint32_t index = 0; index < executable.dataSections.size();
             ++index) {
            const auto &section = executable.dataSections[index];
            if (section.size)
                std::cout << color("1;36", "data" + std::to_string(index))
                          << "  " << formatAddress(section.loadAddress) << " - "
                          << formatAddress(section.loadAddress + section.size)
                          << color("90", "  ") << section.size << " bytes\n";
        }
        if (executable.bssSize)
            std::cout << color("1;33", "bss") << "    "
                      << formatAddress(executable.bssAddress) << " - "
                      << formatAddress(executable.bssAddress +
                                       executable.bssSize)
                      << color("90", "  ") << executable.bssSize << " bytes\n";
    } else if (topic == "exceptions") {
        std::cout << color("1;37", "Exception state") << color("90", "  taken ")
                  << color(cpu.state.exceptionTaken ? "1;31" : "1;32",
                           cpu.state.exceptionTaken ? "yes" : "no")
                  << '\n'
                  << color("90", "srr0  ")
                  << formatAddress(cpu.state.spr[SPR::SRR0])
                  << color("90", "   srr1 ")
                  << formatAddress(cpu.state.spr[SPR::SRR1]) << '\n'
                  << color("90", "dar   ")
                  << formatAddress(cpu.state.spr[SPR::DAR])
                  << color("90", "   dsisr ")
                  << formatAddress(cpu.state.spr[SPR::DSISR]) << '\n'
                  << color("90", "pending  reset=")
                  << cpu.state.systemResetPending
                  << " machine-check=" << cpu.state.machineCheckPending
                  << " external=" << cpu.state.externalInterruptPending
                  << " decrementer=" << cpu.state.decrementerPending << '\n';
    } else {
        error("unknown info topic '" + topic + "'");
    }
}

void Debugger::printHistory() {
    for (uint32_t index = 0; index < history.size(); ++index)
        std::cout << color("90", std::to_string(index + 1)) << "  "
                  << history[index] << '\n';
}

void Debugger::addBreakpoint(const std::vector<std::string> &arguments) {
    if (arguments.size() < 2) {
        error("usage: break <address>");
        return;
    }
    auto address = parseValue(arguments[1]);
    if (!address) {
        error("invalid breakpoint address");
        return;
    }
    uint32_t breakpoint = static_cast<uint32_t>(*address);
    breakpoints.insert(breakpoint);
    std::cout << color("1;32", "breakpoint") << "  "
              << formatAddress(breakpoint) << '\n';
}

void Debugger::removeBreakpoint(const std::vector<std::string> &arguments) {
    if (arguments.size() < 2) {
        error("usage: delete <address|all>");
        return;
    }
    if (lower(arguments[1]) == "all") {
        breakpoints.clear();
        std::cout << color("1;32", "all breakpoints removed") << '\n';
        return;
    }
    auto address = parseValue(arguments[1]);
    if (!address || !breakpoints.erase(static_cast<uint32_t>(*address))) {
        error("breakpoint not found");
        return;
    }
    std::cout << color("1;32", "breakpoint removed") << '\n';
}

void Debugger::addWatchpoint(const std::vector<std::string> &arguments) {
    if (arguments.size() < 2) {
        error("usage: watch <address> [1|2|4|8]");
        return;
    }
    auto address = parseValue(arguments[1]);
    auto size = arguments.size() > 2 ? parseValue(arguments[2])
                                     : std::optional<uint64_t>(4);
    if (!address || !size ||
        (*size != 1 && *size != 2 && *size != 4 && *size != 8)) {
        error("invalid watchpoint address or size");
        return;
    }
    uint32_t location = static_cast<uint32_t>(*address);
    auto value = inspectMemory(location, static_cast<uint32_t>(*size));
    if (!value) {
        error("watchpoint memory is not readable");
        return;
    }
    for (auto &watchpoint : watchpoints) {
        if (watchpoint.address == location) {
            watchpoint = {location, static_cast<uint32_t>(*size), *value};
            std::cout << color("1;32", "watchpoint updated") << "  "
                      << formatAddress(location) << '\n';
            return;
        }
    }
    watchpoints.push_back({location, static_cast<uint32_t>(*size), *value});
    std::cout << color("1;32", "watchpoint") << "  " << formatAddress(location)
              << '\n';
}

void Debugger::removeWatchpoint(const std::vector<std::string> &arguments) {
    if (arguments.size() < 2) {
        error("usage: unwatch <address|all>");
        return;
    }
    if (lower(arguments[1]) == "all") {
        watchpoints.clear();
        std::cout << color("1;32", "all watchpoints removed") << '\n';
        return;
    }
    auto address = parseValue(arguments[1]);
    if (!address) {
        error("invalid watchpoint address");
        return;
    }
    auto found = std::find_if(watchpoints.begin(), watchpoints.end(),
                              [&](const Watchpoint &watchpoint) {
                                  return watchpoint.address ==
                                         static_cast<uint32_t>(*address);
                              });
    if (found == watchpoints.end()) {
        error("watchpoint not found");
        return;
    }
    watchpoints.erase(found);
    std::cout << color("1;32", "watchpoint removed") << '\n';
}

bool Debugger::checkWatchpoints() {
    for (auto &watchpoint : watchpoints) {
        auto value = inspectMemory(watchpoint.address, watchpoint.size);
        if (value && *value != watchpoint.value) {
            uint64_t previous = watchpoint.value;
            watchpoint.value = *value;
            std::cout << color("1;33", "watchpoint hit") << "  "
                      << formatAddress(watchpoint.address) << "  "
                      << color("36", hexValue(previous, watchpoint.size * 2))
                      << color("90", " → ")
                      << color("1;36", hexValue(*value, watchpoint.size * 2))
                      << '\n';
            return true;
        }
    }
    return false;
}

bool Debugger::executeOne(bool display) {
    if (!display) {
        Device::globalDevice->step();
        return cpu.state.exceptionTaken;
    }
    BroadwayState before = cpu.state;
    auto instruction = inspectInstruction(before.cia);
    Device::globalDevice->step();
    if (display) {
        std::cout << color("1;35", "executed") << "  "
                  << formatAddress(before.cia) << "  ";
        if (instruction)
            std::cout << disassemble(*instruction, before.cia);
        else
            std::cout << color("31", "unreadable instruction");
        std::cout << '\n';
        for (uint32_t index = 0; index < 32; ++index) {
            if (before.gpr[index] != cpu.state.gpr[index])
                std::cout << color("1;33", "r" + std::to_string(index))
                          << color("90", "  ")
                          << formatAddress(before.gpr[index])
                          << color("90", " → ")
                          << formatAddress(cpu.state.gpr[index]) << '\n';
        }
        std::array<std::tuple<std::string, uint32_t, uint32_t>, 4> controls = {
            {{"lr", before.spr[SPR::LR], cpu.state.spr[SPR::LR]},
             {"ctr", before.spr[SPR::CTR], cpu.state.spr[SPR::CTR]},
             {"cr", before.cr, cpu.state.cr},
             {"msr", before.msr, cpu.state.msr}}};
        for (const auto &[name, previous, current] : controls) {
            if (previous != current)
                std::cout << color("1;33", name) << color("90", "  ")
                          << formatAddress(previous) << color("90", " → ")
                          << formatAddress(current) << '\n';
        }
        printStatus();
    }
    return cpu.state.exceptionTaken;
}

void Debugger::step(const std::vector<std::string> &arguments) {
    uint32_t count = 1;
    if (arguments.size() > 1) {
        auto parsed = parseValue(arguments[1]);
        if (!parsed || *parsed == 0 || *parsed > 100000) {
            error("step count must be between 1 and 100000");
            return;
        }
        count = static_cast<uint32_t>(*parsed);
    }
    for (uint32_t index = 0; index < count; ++index) {
        bool exception = executeOne(count == 1);
        if (checkWatchpoints())
            break;
        if (exception && stopOnException) {
            std::cout << color("1;31", "exception") << "  "
                      << exceptionName(cpu.state.cia) << " at "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
    }
    if (count != 1)
        printStatus();
}

void Debugger::next() {
    uint32_t origin = cpu.state.cia;
    auto instruction = inspectInstruction(origin);
    if (!instruction) {
        step({"step"});
        return;
    }
    uint32_t op = *instruction >> 26;
    uint32_t xo = (*instruction >> 1) & 1023;
    bool linkedBranch =
        (*instruction & 1) &&
        (op == 18 || op == 16 || (op == 19 && (xo == 16 || xo == 528)));
    if (!linkedBranch) {
        step({"step"});
        return;
    }
    uint32_t returnAddress = origin + 4;
    bool exception = executeOne(false);
    if (checkWatchpoints() || (exception && stopOnException)) {
        if (exception && stopOnException)
            std::cout << color("1;31", "exception") << "  "
                      << exceptionName(cpu.state.cia) << " at "
                      << formatAddress(cpu.state.cia) << '\n';
        printStatus();
        return;
    }
    interrupted = false;
    while (cpu.state.cia != returnAddress) {
        if (interrupted) {
            std::cout << color("1;33", "interrupted") << "  "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
        if (breakpoints.contains(cpu.state.cia)) {
            std::cout << color("1;31", "breakpoint hit") << "  "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
        exception = executeOne(false);
        if (checkWatchpoints())
            break;
        if (exception && stopOnException) {
            std::cout << color("1;31", "exception") << "  "
                      << exceptionName(cpu.state.cia) << " at "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
    }
    printStatus();
}

void Debugger::stepFrame() {
    auto *vi = Device::globalDevice->vi.get();

    if (!vi) {
        error("Video Interface is not initialized");
        return;
    }

    const uint64_t startingFrame = vi->getFrameCounter();
    uint32_t steps = 0;

    interrupted = false;
    bool first = true;

    while (vi->getFrameCounter() == startingFrame) {
        if ((steps++ & 4095) == 0) {
            SDL_Event event;
            while (SDL_PollEvent(&event))
                if (event.type == SDL_EVENT_QUIT)
                    interrupted = true;
        }
        if (interrupted) {
            std::cout << color("1;33", "interrupted") << "  "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }

        if (!first && breakpoints.contains(cpu.state.cia)) {
            std::cout << color("1;31", "breakpoint hit") << "  "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }

        first = false;

        bool exception = executeOne(false);

        if (checkWatchpoints())
            break;

        if (exception && stopOnException) {
            std::cout << color("1;31", "exception") << "  "
                      << exceptionName(cpu.state.cia) << " at "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
    }

    if (vi->getFrameCounter() != startingFrame) {
        std::cout << color("1;32", "frame") << "  " << vi->getFrameCounter()
                  << '\n';
    }

    printStatus();
}

void Debugger::continueExecution() {
    interrupted = false;
    bool first = true;
    while (true) {
        if (interrupted) {
            std::cout << color("1;33", "interrupted") << "  "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
        if (!first && breakpoints.contains(cpu.state.cia)) {
            std::cout << color("1;31", "breakpoint hit") << "  "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
        first = false;
        bool exception = executeOne(false);
        if (checkWatchpoints())
            break;
        if (exception && stopOnException) {
            std::cout << color("1;31", "exception") << "  "
                      << exceptionName(cpu.state.cia) << " at "
                      << formatAddress(cpu.state.cia) << '\n';
            break;
        }
    }
    printStatus();
}

bool Debugger::executeCommand(const std::string &line) {
    auto arguments = tokenize(line);
    if (arguments.empty())
        return true;
    std::string command = lower(arguments[0]);
    try {
        if (command == "h" || command == "help" || command == "?")
            printHelp(arguments);
        else if (command == "s" || command == "step")
            step(arguments);
        else if (command == "n" || command == "next")
            next();
        else if (command == "f" || command == "frame")
            stepFrame();
        else if (command == "c" || command == "continue")
            continueExecution();
        else if (command == "b" || command == "break")
            addBreakpoint(arguments);
        else if (command == "d" || command == "delete")
            removeBreakpoint(arguments);
        else if (command == "watch")
            addWatchpoint(arguments);
        else if (command == "unwatch")
            removeWatchpoint(arguments);
        else if (command == "status" || command == "where")
            printStatus();
        else if (command == "regs")
            printRegisters(arguments);
        else if (command == "reg")
            printRegister(arguments);
        else if (command == "x" || command.starts_with("x/"))
            printMemory(arguments);
        else if (command == "write")
            writeMemory(arguments);
        else if (command == "disasm" || command == "disassemble")
            printDisassembly(arguments);
        else if (command == "info")
            printInfo(arguments);
        else if (command == "history")
            printHistory();
        else if (command == "clear")
            std::cout << "\033[2J\033[H";
        else if (command == "set") {
            if (arguments.size() != 3 ||
                lower(arguments[1]) != "stop-exceptions" ||
                (lower(arguments[2]) != "on" && lower(arguments[2]) != "off")) {
                error("usage: set stop-exceptions <on|off>");
            } else {
                stopOnException = lower(arguments[2]) == "on";
                std::cout << color("1;32", "stop-exceptions") << "  "
                          << (stopOnException ? "on" : "off") << '\n';
            }
        } else if (command == "jump") {
            if (arguments.size() < 2) {
                error("usage: jump <address>");
            } else if (auto address = parseValue(arguments[1])) {
                cpu.state.cia = static_cast<uint32_t>(*address);
                cpu.state.nia = cpu.state.cia + 4;
                printStatus();
            } else {
                error("invalid jump address");
            }
        } else if (command == "q" || command == "quit" || command == "exit") {
            running = false;
        } else {
            error("unknown command '" + arguments[0] + "'");
        }
    } catch (const std::exception &exception) {
        error(exception.what());
        return false;
    }
    return true;
}
