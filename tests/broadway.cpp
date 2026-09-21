#include "device.h"
#include <bit>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr uint32_t CODE = 0x80004000;
constexpr uint32_t DATA = 0x80008000;

uint32_t dtype(uint32_t op, uint32_t d, uint32_t a, int32_t immediate) {
    return (op << 26) | (d << 21) | (a << 16) |
           (static_cast<uint32_t>(immediate) & 0xFFFF);
}

uint32_t xtype(uint32_t op, uint32_t xo, uint32_t d = 3, uint32_t a = 4,
               uint32_t b = 5, bool rc = false) {
    return (op << 26) | (d << 21) | (a << 16) | (b << 11) | (xo << 1) | rc;
}

void expect(uint64_t actual, uint64_t expected, const std::string &field) {
    if (actual != expected)
        throw std::runtime_error(field + ": expected " +
                                 utils::toHexString(expected) + ", got " +
                                 utils::toHexString(actual));
}

void expectFloat(uint64_t actual, double expected, const std::string &field) {
    expect(actual, std::bit_cast<uint64_t>(expected), field);
}

void step(Broadway &cpu, uint32_t instruction) {
    Bus::write32(cpu.state.cia, instruction);
    cpu.executeInstruction();
}

struct BroadwayTest {
    const char *name;
    std::function<void(Broadway &)> run;
};
}

int runBroadwaySuite() {
    auto device = Device::createDevice();
    std::vector<BroadwayTest> tests;
    tests.push_back({"ADDI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 3;
                         step(cpu, dtype(14, 3, 4, 2));
                         expect(cpu.state.gpr[3], 0x5u, "r3");

                         cpu.state.gpr[0] = 99;
                         step(cpu, dtype(14, 3, 0, -1));
                         expect(cpu.state.gpr[3], 0xFFFFFFFF,
                                "r0 means zero for ADDI");
                     }});
    tests.push_back({"ADDIS", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 3;
                         step(cpu, dtype(15, 3, 4, 2));
                         expect(cpu.state.gpr[3], 0x20003u, "r3");
                     }});
    tests.push_back({"MULLI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 3;
                         step(cpu, dtype(7, 3, 4, 2));
                         expect(cpu.state.gpr[3], 0x6u, "r3");
                     }});
    tests.push_back({"SUBFIC", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 3;
                         step(cpu, dtype(8, 3, 4, 2));
                         expect(cpu.state.gpr[3], 0xffffffffu, "r3");
                     }});
    tests.push_back({"ADDIC", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 3;
                         step(cpu, dtype(12, 3, 4, 2));
                         expect(cpu.state.gpr[3], 0x5u, "r3");
                         cpu.state.gpr[4] = 0xFFFFFFFF;
                         step(cpu, dtype(12, 3, 4, 1));
                         expect(cpu.state.gpr[3], 0, "wrapped result");
                         expect(cpu.state.getCA(), 1, "carry");
                     }});
    tests.push_back({"ADDIC.", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 3;
                         step(cpu, dtype(13, 3, 4, 2));
                         expect(cpu.state.gpr[3], 0x5u, "r3");
                         expect(cpu.state.getCRField(0), 4, "CR0");
                         cpu.state.gpr[4] = 0xFFFFFFFF;
                         step(cpu, dtype(13, 3, 4, 1));
                         expect(cpu.state.gpr[3], 0, "wrapped result");
                         expect(cpu.state.getCA(), 1, "carry");
                     }});
    tests.push_back(
        {"ORI", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345600;
             step(cpu, dtype(24, 3, 0, 43264));
             expect(cpu.state.gpr[0], 0x1234ff00u, "r0 destination");
             expect(cpu.state.gpr[3], 0x12345600, "source unchanged");

             cpu.state.gpr[0] = 0x12340000;
             step(cpu, dtype(24, 0, 3, 0x8000));
             expect(cpu.state.gpr[3], 0x12348000, "r0 is a source for ORI");
         }});
    tests.push_back(
        {"ORIS", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345600;
             step(cpu, dtype(25, 3, 0, 32768));
             expect(cpu.state.gpr[0], 0x92345600u, "r0 destination");
             expect(cpu.state.gpr[3], 0x12345600, "source unchanged");
         }});
    tests.push_back(
        {"XORI", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345600;
             step(cpu, dtype(26, 3, 0, 32768));
             expect(cpu.state.gpr[0], 0x1234d600u, "r0 destination");
             expect(cpu.state.gpr[3], 0x12345600, "source unchanged");
         }});
    tests.push_back(
        {"XORIS", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345600;
             step(cpu, dtype(27, 3, 0, 32768));
             expect(cpu.state.gpr[0], 0x92345600u, "r0 destination");
             expect(cpu.state.gpr[3], 0x12345600, "source unchanged");
         }});
    tests.push_back({"ANDI.", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345600;
                         step(cpu, dtype(28, 3, 0, 32768));
                         expect(cpu.state.gpr[0], 0x0u, "r0 destination");
                         expect(cpu.state.gpr[3], 0x12345600,
                                "source unchanged");
                         expect(cpu.state.getCRField(0), 2, "CR0");
                     }});
    tests.push_back({"ANDIS.", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345600;
                         step(cpu, dtype(29, 3, 0, 32768));
                         expect(cpu.state.gpr[0], 0x0u, "r0 destination");
                         expect(cpu.state.gpr[3], 0x12345600,
                                "source unchanged");
                         expect(cpu.state.getCRField(0), 2, "CR0");
                     }});
    tests.push_back({"CMPI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xFFFFFFFF;
                         cpu.state.setSO(true);
                         step(cpu, dtype(11, 12, 4, 1));
                         expect(cpu.state.getCRField(3), 9,
                                "CR3 signedness and SO");
                         expect(cpu.state.getCRField(0), 0, "CR0 unchanged");
                     }});
    tests.push_back({"CMPLI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xFFFFFFFF;
                         cpu.state.setSO(true);
                         step(cpu, dtype(10, 12, 4, 1));
                         expect(cpu.state.getCRField(3), 5,
                                "CR3 signedness and SO");
                         expect(cpu.state.getCRField(0), 0, "CR0 unchanged");
                     }});
    tests.push_back({"TWI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 7;
                         cpu.state.gpr[5] = 7;
                         step(cpu, dtype(3, 4, 4, 7));
                         expect(cpu.state.cia, 0x700, "trap vector");
                         expect(cpu.state.spr[SPR::SRR0], CODE, "saved PC");
                         expect(cpu.state.spr[SPR::SRR1] & 0x20000, 0x20000,
                                "trap cause");
                     }});
    tests.push_back({"TW", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 7;
                         cpu.state.gpr[5] = 7;
                         step(cpu, xtype(31, 4, 4, 4, 5));
                         expect(cpu.state.cia, 0x700, "trap vector");
                         expect(cpu.state.spr[SPR::SRR0], CODE, "saved PC");
                         expect(cpu.state.spr[SPR::SRR1] & 0x20000, 0x20000,
                                "trap cause");
                     }});
    tests.push_back({"ADD", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 266, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xau, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                     }});
    tests.push_back({"ADDO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 778, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xau, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x7fffffffu;
                         cpu.state.gpr[5] = 0x1u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 778, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 778, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"ADDC", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xffffffffu;
                         cpu.state.gpr[5] = 0x1u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 10, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x0u, "result");
                         expect(cpu.state.getCRField(0), 2, "record result");
                         expect(cpu.state.getCA(), 1, "carry");
                     }});
    tests.push_back({"ADDCO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xffffffffu;
                         cpu.state.gpr[5] = 0x1u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 522, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x0u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x7fffffffu;
                         cpu.state.gpr[5] = 0x1u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 522, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 522, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"ADDE", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 138, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xbu, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                         expect(cpu.state.getCA(), 0, "carry");
                     }});
    tests.push_back({"ADDEO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 650, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xbu, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x7fffffffu;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 650, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 650, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"ADDME", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 234, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x7u, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                         expect(cpu.state.getCA(), 1, "carry");
                     }});
    tests.push_back({"ADDMEO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 746, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x7u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 746, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x7fffffffu,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 746, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"ADDZE", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 202, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x8u, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                         expect(cpu.state.getCA(), 0, "carry");
                     }});
    tests.push_back({"ADDZEO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 714, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x8u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x7fffffffu;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 714, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 714, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"SUBF", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 40, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffcu, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                     }});
    tests.push_back({"SUBFO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 552, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffcu, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 552, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 552, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"SUBFC", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 8, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffcu, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                         expect(cpu.state.getCA(), 0, "carry");
                     }});
    tests.push_back({"SUBFCO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 520, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffcu, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 520, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 520, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"SUBFE", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 136, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffcu, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                         expect(cpu.state.getCA(), 0, "carry");
                     }});
    tests.push_back({"SUBFEO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 648, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffcu, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 648, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 648, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"SUBFME", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 232, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffff8u, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                         expect(cpu.state.getCA(), 1, "carry");
                     }});
    tests.push_back({"SUBFMEO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 744, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffff8u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x7fffffffu;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 744, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x7fffffffu,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 744, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"SUBFZE", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 200, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffff9u, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                         expect(cpu.state.getCA(), 0, "carry");
                     }});
    tests.push_back({"SUBFZEO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 712, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffff9u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 712, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 712, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"NEG", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 104, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffff9u, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                     }});
    tests.push_back({"NEGO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 616, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffff9u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 616, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x80000000u,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 616, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"MULLW", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 235, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x15u, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                     }});
    tests.push_back({"MULLWO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0x7u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 747, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x15u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x7fffffffu;
                         cpu.state.gpr[5] = 0x2u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 747, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffeu,
                                "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 747, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"MULHW", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xfffffffeu;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 75, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xffffffffu, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                     }});
    tests.push_back({"MULHWU", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xfffffffeu;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 11, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x2u, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                     }});
    tests.push_back({"DIVW", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xfffffff9u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 491, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffeu, "result");
                         expect(cpu.state.getCRField(0), 8, "record result");
                         cpu.state.gpr[5] = 0;
                         step(cpu, xtype(31, 1003));
                         expect(cpu.state.getOV(), 1,
                                "divide by zero overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                     }});
    tests.push_back({"DIVWO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xfffffff9u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 1003, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0xfffffffeu, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x80000000u;
                         cpu.state.gpr[5] = 0xffffffffu;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 1003, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x0u, "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 1003, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"DIVWU", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xfffffff9u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 459, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x55555553u, "result");
                         expect(cpu.state.getCRField(0), 4, "record result");
                         cpu.state.gpr[5] = 0;
                         step(cpu, xtype(31, 971));
                         expect(cpu.state.getOV(), 1,
                                "divide by zero overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                     }});
    tests.push_back({"DIVWUO", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xfffffff9u;
                         cpu.state.gpr[5] = 0x3u;
                         cpu.state.setCA(true);
                         step(cpu, xtype(31, 971, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x55555553u, "OE result");
                         expect(cpu.state.getOV(), 0, "no signed overflow");

                         cpu.state.gpr[4] = 0x1u;
                         cpu.state.gpr[5] = 0x0u;
                         cpu.state.setCA(false);
                         step(cpu, xtype(31, 971, 3, 4, 5, true));
                         expect(cpu.state.gpr[3], 0x0u, "overflow result");
                         expect(cpu.state.getOV(), 1, "signed overflow");
                         expect(cpu.state.getSO(), 1, "sticky overflow");
                         cpu.state.gpr[4] = 0;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 971, 3, 4, 5, true));
                         expect(cpu.state.getOV(), 0, "overflow cleared");
                         expect(cpu.state.getSO(), 1, "SO stays set");
                     }});
    tests.push_back({"AND", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 28, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xf000f0u, "result");
                         expect(cpu.state.getCRField(0), 4, "CR0");
                     }});
    tests.push_back({"ANDC", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 60, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xf000f000u, "result");
                         expect(cpu.state.getCRField(0), 8, "CR0");
                     }});
    tests.push_back({"OR", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 444, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xfff0fff0u, "result");
                         expect(cpu.state.getCRField(0), 8, "CR0");
                     }});
    tests.push_back({"NOR", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 124, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xf000fu, "result");
                         expect(cpu.state.getCRField(0), 4, "CR0");
                     }});
    tests.push_back({"XOR", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 316, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xff00ff00u, "result");
                         expect(cpu.state.getCRField(0), 8, "CR0");
                     }});
    tests.push_back({"ORC", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 412, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xf0fff0ffu, "result");
                         expect(cpu.state.getCRField(0), 8, "CR0");
                     }});
    tests.push_back({"NAND", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 476, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xff0fff0fu, "result");
                         expect(cpu.state.getCRField(0), 8, "CR0");
                     }});
    tests.push_back({"EQV", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0xF0F0F0F0;
                         cpu.state.gpr[5] = 0x0FF00FF0;
                         step(cpu, xtype(31, 284, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xff00ffu, "result");
                         expect(cpu.state.getCRField(0), 4, "CR0");
                     }});
    tests.push_back({"CNTLZW", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x100u;
                         cpu.state.gpr[5] = 0;
                         step(cpu, xtype(31, 26, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0x17u, "result");
                     }});
    tests.push_back({"EXTSB", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x80u;
                         cpu.state.gpr[5] = 0;
                         step(cpu, xtype(31, 954, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xffffff80u, "result");
                     }});
    tests.push_back({"EXTSH", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x8000u;
                         cpu.state.gpr[5] = 0;
                         step(cpu, xtype(31, 922, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xffff8000u, "result");
                     }});
    tests.push_back({"SLW", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x80000001u;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 24, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0x2u, "result");
                         cpu.state.gpr[5] = 32;
                         step(cpu, xtype(31, 24));
                         expect(cpu.state.gpr[4], 0, "shift by 32");
                     }});
    tests.push_back({"SRW", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x80000001u;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 536, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0x40000000u, "result");
                         cpu.state.gpr[5] = 32;
                         step(cpu, xtype(31, 536));
                         expect(cpu.state.gpr[4], 0, "shift by 32");
                     }});
    tests.push_back({"SRAW", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x80000001u;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 792, 3, 4, 5, true));
                         expect(cpu.state.gpr[4], 0xc0000000u, "result");
                         expect(cpu.state.getCA(), 1, "shift carry");
                         cpu.state.gpr[5] = 32;
                         step(cpu, xtype(31, 792));
                         expect(cpu.state.gpr[4], 0xFFFFFFFF, "shift by 32");
                     }});
    tests.push_back({"SRAWI", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x80000001u;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 824, 3, 4, 1, true));
                         expect(cpu.state.gpr[4], 0xc0000000u, "result");
                         expect(cpu.state.getCA(), 1, "shift carry");
                     }});
    tests.push_back({"CMP", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xFFFFFFFF;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 0, 12));
                         expect(cpu.state.getCRField(3), 8, "CR3");
                     }});
    tests.push_back({"CMPL", [](Broadway &cpu) {
                         cpu.state.gpr[4] = 0xFFFFFFFF;
                         cpu.state.gpr[5] = 1;
                         step(cpu, xtype(31, 32, 12));
                         expect(cpu.state.getCRField(3), 4, "CR3");
                     }});
    tests.push_back({"RLWIMI", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[4] = 0xAAAAAAAA;
                         cpu.state.gpr[5] = 8;
                         step(cpu, (20u << 26) | (3 << 21) | (4 << 16) |
                                       (8 << 11) | (8 << 6) | (23 << 1) | 1);
                         expect(cpu.state.gpr[4], 0xaa5678aa,
                                "masked rotation");
                     }});
    tests.push_back({"RLWINM", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[4] = 0xAAAAAAAA;
                         cpu.state.gpr[5] = 8;
                         step(cpu, (21u << 26) | (3 << 21) | (4 << 16) |
                                       (8 << 11) | (8 << 6) | (23 << 1) | 1);
                         expect(cpu.state.gpr[4], 0x567800, "masked rotation");

                         cpu.state.gpr[3] = 0x12345678;
                         step(cpu, (21u << 26) | (3 << 21) | (4 << 16) |
                                       (28 << 6) | (3 << 1));
                         expect(cpu.state.gpr[4], 0x10000008,
                                "wrapping mask and zero shift");
                     }});
    tests.push_back({"RLWNM", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[4] = 0xAAAAAAAA;
                         cpu.state.gpr[5] = 8;
                         step(cpu, (23u << 26) | (3 << 21) | (4 << 16) |
                                       (5 << 11) | (8 << 6) | (23 << 1) | 1);
                         expect(cpu.state.gpr[4], 0x567800, "masked rotation");
                     }});
    tests.push_back({"LWZ", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write32(DATA, 0x81234567);
                         step(cpu, dtype(32, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0x81234567u, "load result");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"LWZU", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write32(DATA, 0x81234567);
                         step(cpu, dtype(33, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0x81234567u, "load result");
                         expect(cpu.state.gpr[4], DATA, "base register");

                         uint32_t before = cpu.state.gpr[3];
                         step(cpu, dtype(33, 3, 3, 0));
                         expect(cpu.state.cia, 0x700, "invalid update trap");
                         expect(cpu.state.gpr[3], before,
                                "invalid update leaves destination unchanged");
                     }});
    tests.push_back({"LBZ", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write8(DATA, 0x81);
                         step(cpu, dtype(34, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0x81u, "load result");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"LBZU", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write8(DATA, 0x81);
                         step(cpu, dtype(35, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0x81u, "load result");
                         expect(cpu.state.gpr[4], DATA, "base register");
                     }});
    tests.push_back({"LHZ", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, dtype(40, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0x8123u, "load result");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"LHZU", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, dtype(41, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0x8123u, "load result");
                         expect(cpu.state.gpr[4], DATA, "base register");
                     }});
    tests.push_back({"LHA", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, dtype(42, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0xffff8123u, "load result");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"LHAU", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 4;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, dtype(43, 3, 4, -4));
                         expect(cpu.state.gpr[3], 0xffff8123u, "load result");
                         expect(cpu.state.gpr[4], DATA, "base register");
                     }});
    tests.push_back({"STW", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x81234567;
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(36, 3, 4, -4));
                         expect(Bus::read32(DATA), 0x81234567, "stored value");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"STWU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x81234567;
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(37, 3, 4, -4));
                         expect(Bus::read32(DATA), 0x81234567, "stored value");
                         expect(cpu.state.gpr[4], DATA, "base register");
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(37, 4, 4, -4));
                         expect(Bus::read32(DATA), DATA + 4,
                                "same source and base");
                     }});
    tests.push_back({"STB", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x81234567;
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(38, 3, 4, -4));
                         expect(Bus::read8(DATA), 0x67, "stored value");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"STBU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x81234567;
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(39, 3, 4, -4));
                         expect(Bus::read8(DATA), 0x67, "stored value");
                         expect(cpu.state.gpr[4], DATA, "base register");
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(39, 4, 4, -4));
                         expect(Bus::read8(DATA), 4, "same source and base");
                     }});
    tests.push_back({"STH", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x81234567;
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(44, 3, 4, -4));
                         expect(Bus::read16(DATA), 0x4567, "stored value");
                         expect(cpu.state.gpr[4], DATA + 4, "base register");
                     }});
    tests.push_back({"STHU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x81234567;
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(45, 3, 4, -4));
                         expect(Bus::read16(DATA), 0x4567, "stored value");
                         expect(cpu.state.gpr[4], DATA, "base register");
                         cpu.state.gpr[4] = DATA + 4;
                         step(cpu, dtype(45, 4, 4, -4));
                         expect(Bus::read16(DATA), 0x8004,
                                "same source and base");
                     }});
    tests.push_back({"LMW", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         Bus::write32(DATA, 0x12345678);
                         Bus::write32(DATA + 4, 0xABCDEF01);
                         step(cpu, dtype(46, 30, 4, 0));
                         expect(cpu.state.gpr[30], 0x12345678, "first word");
                         expect(cpu.state.gpr[31], 0xABCDEF01, "last word");
                     }});
    tests.push_back({"STMW", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         cpu.state.gpr[30] = 0x12345678;
                         cpu.state.gpr[31] = 0xABCDEF01;
                         step(cpu, dtype(47, 30, 4, 0));
                         expect(Bus::read32(DATA), 0x12345678, "first word");
                         expect(Bus::read32(DATA + 4), 0xABCDEF01, "last word");
                     }});
    tests.push_back({"LFS", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         Bus::write32(DATA, 0xC0200000);
                         step(cpu, dtype(48, 3, 3, -8));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating load");
                         expect(cpu.state.gpr[3], DATA + 8, "base register");
                         expectFloat(cpu.state.ps1[3], -2.5,
                                     "replicated single");
                     }});
    tests.push_back({"LFSU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         Bus::write32(DATA, 0xC0200000);
                         step(cpu, dtype(49, 3, 3, -8));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating load");
                         expect(cpu.state.gpr[3], DATA, "base register");
                         expectFloat(cpu.state.ps1[3], -2.5,
                                     "replicated single");
                     }});
    tests.push_back({"LFD", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         Bus::write64(DATA, 0xC004000000000000ull);
                         step(cpu, dtype(50, 3, 3, -8));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating load");
                         expect(cpu.state.gpr[3], DATA + 8, "base register");
                     }});
    tests.push_back({"LFDU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         Bus::write64(DATA, 0xC004000000000000ull);
                         step(cpu, dtype(51, 3, 3, -8));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating load");
                         expect(cpu.state.gpr[3], DATA, "base register");
                     }});
    tests.push_back({"STFS", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, dtype(52, 3, 3, -8));
                         expect(Bus::read32(DATA), 0xC0200000, "single store");
                         expect(cpu.state.gpr[3], DATA + 8, "base register");

                         cpu.state.gpr[4] = DATA;
                         cpu.state.fpr[3] = 0x3FF000001FFFFFFFull;
                         step(cpu, dtype(52, 3, 4, 0));
                         expect(Bus::read32(DATA), 0x3F800000,
                                "store truncates mantissa");
                     }});
    tests.push_back({"STFSU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, dtype(53, 3, 3, -8));
                         expect(Bus::read32(DATA), 0xC0200000, "single store");
                         expect(cpu.state.gpr[3], DATA, "base register");
                     }});
    tests.push_back({"STFD", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, dtype(54, 3, 3, -8));
                         expect(Bus::read64(DATA), 0xC004000000000000ull,
                                "double store");
                         expect(cpu.state.gpr[3], DATA + 8, "base register");
                     }});
    tests.push_back({"STFDU", [](Broadway &cpu) {
                         cpu.state.gpr[3] = DATA + 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, dtype(55, 3, 3, -8));
                         expect(Bus::read64(DATA), 0xC004000000000000ull,
                                "double store");
                         expect(cpu.state.gpr[3], DATA, "base register");
                     }});
    tests.push_back({"LWZX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write32(DATA, 0x81234567);
                         step(cpu, xtype(31, 23));
                         expect(cpu.state.gpr[3], 0x81234567u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LWZUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write32(DATA, 0x81234567);
                         step(cpu, xtype(31, 55));
                         expect(cpu.state.gpr[3], 0x81234567u, "indexed load");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"LBZX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write8(DATA, 0x81);
                         step(cpu, xtype(31, 87));
                         expect(cpu.state.gpr[3], 0x81u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LBZUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write8(DATA, 0x81);
                         step(cpu, xtype(31, 119));
                         expect(cpu.state.gpr[3], 0x81u, "indexed load");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"LHZX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, xtype(31, 279));
                         expect(cpu.state.gpr[3], 0x8123u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LHZUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, xtype(31, 311));
                         expect(cpu.state.gpr[3], 0x8123u, "indexed load");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"LHAX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, xtype(31, 343));
                         expect(cpu.state.gpr[3], 0xffff8123u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LHAUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, xtype(31, 375));
                         expect(cpu.state.gpr[3], 0xffff8123u, "indexed load");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"STWX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 151));
                         expect(Bus::read32(DATA), 0x81234567u,
                                "indexed store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STWUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 183));
                         expect(Bus::read32(DATA), 0x81234567u,
                                "indexed store");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"STBX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 215));
                         expect(Bus::read8(DATA), 0x67u, "indexed store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STBUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 247));
                         expect(Bus::read8(DATA), 0x67u, "indexed store");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"STHX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 407));
                         expect(Bus::read16(DATA), 0x4567u, "indexed store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STHUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 439));
                         expect(Bus::read16(DATA), 0x4567u, "indexed store");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"LWBRX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write32(DATA, 0x81234567);
                         step(cpu, xtype(31, 534));
                         expect(cpu.state.gpr[3], 0x67452381u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LHBRX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write16(DATA, 0x8123);
                         step(cpu, xtype(31, 790));
                         expect(cpu.state.gpr[3], 0x2381u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STWBRX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 662));
                         expect(Bus::read32(DATA), 0x67452381u,
                                "indexed store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STHBRX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 918));
                         expect(Bus::read16(DATA), 0x6745u, "indexed store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LFSX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write32(DATA, 0xC0200000);
                         step(cpu, xtype(31, 535));
                         expectFloat(cpu.state.fpr[3], -2.5,
                                     "indexed floating load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LFSUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write32(DATA, 0xC0200000);
                         step(cpu, xtype(31, 567));
                         expectFloat(cpu.state.fpr[3], -2.5,
                                     "indexed floating load");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"LFDX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write64(DATA, 0xC004000000000000ull);
                         step(cpu, xtype(31, 599));
                         expectFloat(cpu.state.fpr[3], -2.5,
                                     "indexed floating load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LFDUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         Bus::write64(DATA, 0xC004000000000000ull);
                         step(cpu, xtype(31, 631));
                         expectFloat(cpu.state.fpr[3], -2.5,
                                     "indexed floating load");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"STFSX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, xtype(31, 663));
                         expect(Bus::read32(DATA), 0xC0200000,
                                "indexed floating store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STFSUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, xtype(31, 695));
                         expect(Bus::read32(DATA), 0xC0200000,
                                "indexed floating store");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"STFDX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, xtype(31, 727));
                         expect(Bus::read64(DATA), 0xC004000000000000ull,
                                "indexed floating store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"STFDUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, xtype(31, 759));
                         expect(Bus::read64(DATA), 0xC004000000000000ull,
                                "indexed floating store");
                         expect(cpu.state.gpr[4], DATA, "indexed base");
                     }});
    tests.push_back({"STFIWX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.fpr[3] = 0xFFF8000012345678ull;
                         step(cpu, xtype(31, 983));
                         expect(Bus::read32(DATA), 0x12345678,
                                "integer word store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"ECIWX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.spr[282] = 0x80000000;
                         Bus::write32(DATA, 0x81234567);
                         step(cpu, xtype(31, 310));
                         expect(cpu.state.gpr[3], 0x81234567u, "indexed load");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"ECOWX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.spr[282] = 0x80000000;
                         cpu.state.gpr[3] = 0x81234567;
                         step(cpu, xtype(31, 438));
                         expect(Bus::read32(DATA), 0x81234567u,
                                "indexed store");
                         expect(cpu.state.gpr[4], DATA - 8, "indexed base");
                     }});
    tests.push_back({"LSWX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         cpu.state.gpr[5] = 0;
                         cpu.state.spr[SPR::XER] = 5;
                         Bus::write32(DATA, 0x12345678);
                         Bus::write8(DATA + 4, 0xAB);
                         step(cpu, xtype(31, 533, 31, 4, 5));
                         expect(cpu.state.gpr[31], 0x12345678,
                                "first register");
                         expect(cpu.state.gpr[0], 0xAB000000,
                                "register wrap and partial word");
                     }});
    tests.push_back({"LSWI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         cpu.state.gpr[5] = 0;
                         cpu.state.spr[SPR::XER] = 5;
                         Bus::write32(DATA, 0x12345678);
                         Bus::write8(DATA + 4, 0xAB);
                         step(cpu, xtype(31, 597, 31, 4, 5));
                         expect(cpu.state.gpr[31], 0x12345678,
                                "first register");
                         expect(cpu.state.gpr[0], 0xAB000000,
                                "register wrap and partial word");
                     }});
    tests.push_back({"STSWX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         cpu.state.gpr[5] = 0;
                         cpu.state.spr[SPR::XER] = 5;
                         cpu.state.gpr[31] = 0x12345678;
                         cpu.state.gpr[0] = 0xABCDEF01;
                         step(cpu, xtype(31, 661, 31, 4, 5));
                         expect(Bus::read32(DATA), 0x12345678, "first word");
                         expect(Bus::read8(DATA + 4), 0xAB, "partial word");
                     }});
    tests.push_back({"STSWI", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         cpu.state.gpr[5] = 0;
                         cpu.state.spr[SPR::XER] = 5;
                         cpu.state.gpr[31] = 0x12345678;
                         cpu.state.gpr[0] = 0xABCDEF01;
                         step(cpu, xtype(31, 725, 31, 4, 5));
                         expect(Bus::read32(DATA), 0x12345678, "first word");
                         expect(Bus::read8(DATA + 4), 0xAB, "partial word");
                     }});
    tests.push_back({"LWARX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA;
                         Bus::write32(DATA, 0x12345678);
                         step(cpu, xtype(31, 20, 3, 4, 0));
                         expect(cpu.state.gpr[3], 0x12345678, "reserved load");
                         expect(cpu.state.reservationValid, 1, "reservation");
                         expect(cpu.state.reservationAddress, DATA,
                                "reservation address");
                     }});
    tests.push_back(
        {"STWCX.", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.gpr[3] = 0x12345678;
             cpu.state.reservationValid = true;
             cpu.state.reservationAddress = DATA;
             step(cpu, xtype(31, 150, 3, 4, 0, true));
             expect(Bus::read32(DATA), 0x12345678, "conditional store");
             expect(cpu.state.getCRField(0), 2, "successful reservation");
             expect(cpu.state.reservationValid, 0, "reservation consumed");
             cpu.state.gpr[3] = 0;
             step(cpu, xtype(31, 150, 3, 4, 0, true));
             expect(cpu.state.getCRField(0), 0, "failed reservation");
             expect(Bus::read32(DATA), 0x12345678,
                    "failed store preserves memory");

             cpu.state.reservationValid = true;
             cpu.state.reservationAddress = DATA;
             Bus::write8(DATA + 1, 0x99);
             step(cpu, xtype(31, 150, 3, 4, 0, true));
             expect(cpu.state.getCRField(0), 0,
                    "intervening store invalidates reservation");
         }});
    tests.push_back({"B", [](Broadway &cpu) {
                         step(cpu, (18u << 26) | 0x10 | 1);
                         expect(cpu.state.cia, CODE + 16, "relative target");
                         expect(cpu.state.spr[SPR::LR], CODE + 4, "link");
                         Bus::write32(CODE + 16, dtype(14, 3, 0, 42));
                         cpu.executeInstruction();
                         expect(cpu.state.gpr[3], 42, "fetch follows branch");
                         step(cpu, (18u << 26) | 0x03FFFFFC);
                         expect(cpu.state.cia, CODE + 16,
                                "negative displacement");

                         step(cpu, (18u << 26) | 0x100 | 2);
                         expect(cpu.state.cia, 0x100, "absolute target");
                     }});
    tests.push_back({"BC", [](Broadway &cpu) {
                         cpu.state.spr[SPR::CTR] = 2;
                         step(cpu, (16u << 26) | (16 << 21) | 0x10);
                         expect(cpu.state.cia, CODE + 16, "counted branch");
                         expect(cpu.state.spr[SPR::CTR], 1, "decrement CTR");
                         step(cpu, (16u << 26) | (16 << 21) | 0x10 | 1);
                         expect(cpu.state.cia, CODE + 20, "not taken");
                         expect(cpu.state.spr[SPR::LR], CODE + 20,
                                "link even when not taken");
                         cpu.state.cr = 0x20000000;
                         step(cpu,
                              (16u << 26) | (12 << 21) | (2 << 16) | 0xFFFC);
                         expect(cpu.state.cia, CODE + 16,
                                "CR condition and negative displacement");
                     }});
    tests.push_back({"BCLR", [](Broadway &cpu) {
                         cpu.state.spr[SPR::LR] = CODE + 19;
                         step(cpu, xtype(19, 16, 20, 0, 0, true));
                         expect(cpu.state.cia, CODE + 16,
                                "register target alignment");
                         expect(cpu.state.spr[SPR::LR], CODE + 4,
                                "link after reading target");
                     }});
    tests.push_back({"BCCTR", [](Broadway &cpu) {
                         cpu.state.spr[SPR::CTR] = CODE + 19;
                         step(cpu, xtype(19, 528, 20, 0, 0, true));
                         expect(cpu.state.cia, CODE + 16,
                                "register target alignment");
                         expect(cpu.state.spr[SPR::LR], CODE + 4,
                                "link after reading target");
                     }});
    tests.push_back({"SC", [](Broadway &cpu) {
                         cpu.state.msr = 0xF032;
                         step(cpu, (17u << 26) | 2);
                         expect(cpu.state.cia, 0xC00, "system call vector");
                         expect(cpu.state.spr[SPR::SRR0], CODE + 4,
                                "return PC");
                         expect(cpu.state.spr[SPR::SRR1], 0xF032, "saved MSR");
                         expect(cpu.state.msr & 0xE032, 0, "exception MSR");
                     }});
    tests.push_back({"RFI", [](Broadway &cpu) {
                         cpu.state.spr[SPR::SRR0] = CODE + 19;
                         cpu.state.spr[SPR::SRR1] = 0xF032;
                         step(cpu, xtype(19, 50, 0, 0, 0));
                         expect(cpu.state.cia, CODE + 16, "return target");
                         expect(cpu.state.msr, 0xF032, "restored MSR");
                     }});
    tests.push_back(
        {"CRAND", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 257, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 0, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CRANDC", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 129, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 1, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CREQV", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 289, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 0, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CRNAND", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 225, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 1, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CRNOR", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 33, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 0, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CROR", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 449, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 1, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CRORC", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 417, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 1, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back(
        {"CRXOR", [](Broadway &cpu) {
             cpu.state.cr = 1u << 27;
             step(cpu, xtype(19, 193, 3, 4, 5));
             expect((cpu.state.cr >> 28) & 1, 1, "CR boolean result");
             expect((cpu.state.cr >> 27) & 1, 1, "source bit preserved");
         }});
    tests.push_back({"MCRF", [](Broadway &cpu) {
                         cpu.state.setCRField(4, 9);
                         step(cpu, xtype(19, 0, 12, 16, 0));
                         expect(cpu.state.getCRField(3), 9, "CR field copy");
                     }});
    tests.push_back({"MCRXR", [](Broadway &cpu) {
                         cpu.state.spr[SPR::XER] = 0xE0000055;
                         step(cpu, xtype(31, 512, 12, 0, 0));
                         expect(cpu.state.getCRField(3), 14, "XER flags");
                         expect(cpu.state.spr[SPR::XER], 0x55,
                                "XER flags cleared");
                     }});
    tests.push_back({"MFCR", [](Broadway &cpu) {
                         cpu.state.cr = 0x12345678;
                         step(cpu, xtype(31, 19, 3, 0, 0));
                         expect(cpu.state.gpr[3], 0x12345678, "CR read");
                     }});
    tests.push_back({"MTCRF", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.cr = 0xFFFFFFFF;
                         step(cpu, (31u << 26) | (3 << 21) | (0x81 << 12) |
                                       (144 << 1));
                         expect(cpu.state.cr, 0x1FFFFFF8, "selected CR fields");
                     }});
    tests.push_back({"MFMSR", [](Broadway &cpu) {
                         cpu.state.msr = 0x2030;
                         step(cpu, xtype(31, 83, 3, 0, 0));
                         expect(cpu.state.gpr[3], 0x2030, "MSR read");

                         cpu.state.msr |= 0x4000;
                         step(cpu, xtype(31, 83, 3, 0, 0));
                         expect(cpu.state.cia, 0x700,
                                "privileged instruction trap");
                         expect(cpu.state.spr[SPR::SRR1] & 0x40000, 0x40000,
                                "privilege cause");
                     }});
    tests.push_back({"MTMSR", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x2030;
                         step(cpu, xtype(31, 146, 3, 0, 0));
                         expect(cpu.state.msr, 0x2030, "MSR write");
                     }});
    tests.push_back(
        {"MFSPR", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345678;
             cpu.state.spr[912] = 0xABCDEF01;
             step(cpu, (31u << 26) | (3 << 21) | 1105920u | (339 << 1));
             expect(cpu.state.gpr[3], 0xABCDEF01, "split SPR encoding");
         }});
    tests.push_back(
        {"MTSPR", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345678;
             cpu.state.spr[912] = 0xABCDEF01;
             step(cpu, (31u << 26) | (3 << 21) | 1105920u | (467 << 1));
             expect(cpu.state.spr[912], 0x12345678, "split SPR encoding");
         }});
    tests.push_back(
        {"MFTB", [](Broadway &cpu) {
             cpu.state.gpr[3] = 0x12345678;
             cpu.state.spr[268] = 0xABCDEF01;
             step(cpu, (31u << 26) | (3 << 21) | 802816u | (371 << 1));
             expect(cpu.state.gpr[3], 0xABCDEF01, "split SPR encoding");
         }});
    tests.push_back({"MFSR", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[5] = 0x60000000;
                         cpu.state.sr[6] = 0xABCDEF01;
                         step(cpu, xtype(31, 595, 3, 6, 5));
                         expect(cpu.state.gpr[3], 0xABCDEF01,
                                "segment register");
                     }});
    tests.push_back({"MTSR", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[5] = 0x60000000;
                         cpu.state.sr[6] = 0xABCDEF01;
                         step(cpu, xtype(31, 210, 3, 6, 5));
                         expect(cpu.state.sr[6], 0x12345678,
                                "segment register");
                     }});
    tests.push_back({"MFSRIN", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[5] = 0x60000000;
                         cpu.state.sr[6] = 0xABCDEF01;
                         step(cpu, xtype(31, 659, 3, 6, 5));
                         expect(cpu.state.gpr[3], 0xABCDEF01,
                                "segment register");
                     }});
    tests.push_back({"MTSRIN", [](Broadway &cpu) {
                         cpu.state.gpr[3] = 0x12345678;
                         cpu.state.gpr[5] = 0x60000000;
                         cpu.state.sr[6] = 0xABCDEF01;
                         step(cpu, xtype(31, 242, 3, 6, 5));
                         expect(cpu.state.sr[6], 0x12345678,
                                "segment register");
                     }});
    tests.push_back(
        {"DCBST", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 54, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"DCBF", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 86, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"DCBTST", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 246, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"DCBT", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 278, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"DCBI", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 470, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"DCBA", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 758, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"SYNC", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 598, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"EIEIO", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 854, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"ICBI", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 982, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"TLBIE", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 306, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"TLBSYNC", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(31, 566, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back(
        {"ISYNC", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA;
             cpu.state.cr = 0x12345678;
             Bus::write32(DATA, 0xABCDEF01);
             step(cpu, xtype(19, 150, 0, 4, 0));
             expect(cpu.state.cia, CODE + 4, "instruction completion");
             expect(cpu.state.cr, 0x12345678, "CR preserved");
             expect(Bus::read32(DATA), 0xABCDEF01, "memory preserved");
         }});
    tests.push_back({"DCBZ", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 13;
                         for (uint32_t i = 0; i < 40; i += 4)
                             Bus::write32(DATA + i, 0xFFFFFFFF);
                         step(cpu, xtype(31, 1014, 0, 4, 0));
                         for (uint32_t i = 0; i < 32; i += 4)
                             expect(Bus::read32(DATA + i), 0,
                                    "zeroed cache block");
                         expect(Bus::read32(DATA + 32), 0xFFFFFFFF,
                                "adjacent block unchanged");
                     }});
    tests.push_back({"DCBZ_L", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA + 13;
                         for (uint32_t i = 0; i < 40; i += 4)
                             Bus::write32(DATA + i, 0xFFFFFFFF);
                         step(cpu, xtype(4, 1014, 0, 4, 0));
                         for (uint32_t i = 0; i < 32; i += 4)
                             expect(Bus::read32(DATA + i), 0,
                                    "zeroed cache block");
                         expect(Bus::read32(DATA + 32), 0xFFFFFFFF,
                                "adjacent block unchanged");
                     }});
    tests.push_back(
        {"FADD", [](Broadway &cpu) {
             cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
             cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
             cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
             cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
             step(cpu, xtype(63, 21, 3, 4, 5, true) | (6 << 6));
             expectFloat(cpu.state.fpr[3], 10.0, "floating result");
             expectFloat(cpu.state.ps1[3], 99.0, "second lane");
             expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
             expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                    "CR1 record");

             cpu.state.fpr[4] = 0x7FF0000000000000ull;
             cpu.state.fpr[5] = 0xFFF0000000000000ull;
             cpu.state.fpr[3] = std::bit_cast<uint64_t>(42.0);
             cpu.state.fpscr = 0x80;
             step(cpu, xtype(63, 21));
             expectFloat(cpu.state.fpr[3], 42.0,
                         "enabled invalid suppresses result");
             expect(cpu.state.fpscr & 0xE0800000, 0xE0800000,
                    "invalid exception flags");
             cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
             cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
             step(cpu, xtype(63, 21));
             expectFloat(cpu.state.fpr[3], 3.0,
                         "old sticky invalid does not suppress valid result");
             cpu.state.msr = 0;
             step(cpu, xtype(63, 21));
             expect(cpu.state.cia, 0x800, "FPU unavailable exception");
         }});
    tests.push_back({"FADDS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 21, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 10.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 10.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FSUB", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 20, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 6.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FSUBS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 20, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 6.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 6.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FDIV", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 18, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 4.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");

                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(-0.0);
                         step(cpu, xtype(63, 18));
                         expect(cpu.state.fpr[3], 0xFFF0000000000000ull,
                                "signed infinity");
                         expect(cpu.state.fpscr & 0x84000000, 0x84000000,
                                "divide by zero flags");
                     }});
    tests.push_back({"FDIVS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 18, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 4.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 4.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FMUL", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 25, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 24.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FMULS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 25, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 24.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 24.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FMADD", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 29, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 26.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FMADDS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 29, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 26.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 26.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FMSUB", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 28, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 22.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FMSUBS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 28, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 22.0, "floating result");
                         expectFloat(cpu.state.ps1[3], 22.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 4, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FNMADD", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 31, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], -26.0,
                                     "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 8, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FNMADDS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 31, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], -26.0,
                                     "floating result");
                         expectFloat(cpu.state.ps1[3], -26.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 8, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FNMSUB", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(63, 30, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], -22.0,
                                     "floating result");
                         expectFloat(cpu.state.ps1[3], 99.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 8, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FNMSUBS", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(99.0);
                         step(cpu, xtype(59, 30, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], -22.0,
                                     "floating result");
                         expectFloat(cpu.state.ps1[3], -22.0, "second lane");
                         expect((cpu.state.fpscr >> 12) & 31, 8, "FPRF");
                         expect(cpu.state.getCRField(1), cpu.state.fpscr >> 28,
                                "CR1 record");
                     }});
    tests.push_back({"FRES", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(59, 24, 3, 0, 5, true));
                         expect(cpu.state.fpr[3], 0x3FCFFF0000000000ull,
                                "floating result");
                     }});
    tests.push_back({"FRSQRTE", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(63, 26, 3, 0, 5, true));
                         expect(cpu.state.fpr[3], 0x3FDFFE8000000000ull,
                                "floating result");
                     }});
    tests.push_back({"FRSP", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.5);
                         step(cpu, xtype(63, 12, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], 2.5, "floating result");

                         cpu.state.fpscr = 4;
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(0x1p-130);
                         step(cpu, xtype(63, 12, 3, 0, 5));
                         expectFloat(cpu.state.fpr[3], 0.0,
                                     "NI flushes single subnormals");
                     }});
    tests.push_back({"FNEG", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.5);
                         step(cpu, xtype(63, 40, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating result");
                     }});
    tests.push_back({"FABS", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, xtype(63, 264, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], 2.5, "floating result");
                     }});
    tests.push_back({"FNABS", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.5);
                         step(cpu, xtype(63, 136, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating result");
                     }});
    tests.push_back({"FMR", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(-2.5);
                         step(cpu, xtype(63, 72, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], -2.5, "floating result");
                     }});
    tests.push_back({"FCTIW", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.75);
                         step(cpu, xtype(63, 14, 3, 0, 5, true));
                         expect(static_cast<uint32_t>(cpu.state.fpr[3]), 4,
                                "integer conversion");
                         expect(cpu.state.fpscr & 0x02000000, 0x02000000,
                                "inexact conversion");

                         cpu.state.fpscr = 3;
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(-3.25);
                         step(cpu, xtype(63, 14, 3, 0, 5));
                         expect(static_cast<uint32_t>(cpu.state.fpr[3]),
                                0xFFFFFFFC, "round toward negative infinity");
                     }});
    tests.push_back({"FCTIWZ", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.75);
                         step(cpu, xtype(63, 15, 3, 0, 5, true));
                         expect(static_cast<uint32_t>(cpu.state.fpr[3]), 3,
                                "integer conversion");
                         expect(cpu.state.fpscr & 0x02000000, 0x02000000,
                                "inexact conversion");

                         cpu.state.fpscr = 0;
                         cpu.state.fpr[5] = 0x7FF8000000000000ull;
                         step(cpu, xtype(63, 15, 3, 0, 5));
                         expect(static_cast<uint32_t>(cpu.state.fpr[3]),
                                0x80000000, "NaN integer conversion");
                         expect(cpu.state.fpscr & 0x100, 0x100,
                                "invalid conversion flag");
                     }});
    tests.push_back(
        {"FCMPU", [](Broadway &cpu) {
             cpu.state.fpr[4] = std::bit_cast<uint64_t>(-2.0);
             cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.0);
             step(cpu, xtype(63, 0, 12, 4, 5));
             expect(cpu.state.getCRField(3), 8, "floating comparison");
             cpu.state.fpr[4] = 0x7FF8000000000001ull;
             step(cpu, xtype(63, 0, 12, 4, 5));
             expect(cpu.state.getCRField(3), 1, "unordered comparison");
             expect(cpu.state.fpscr & 0x00080000, 0x0, "ordered NaN exception");
         }});
    tests.push_back(
        {"FCMPO", [](Broadway &cpu) {
             cpu.state.fpr[4] = std::bit_cast<uint64_t>(-2.0);
             cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.0);
             step(cpu, xtype(63, 32, 12, 4, 5));
             expect(cpu.state.getCRField(3), 8, "floating comparison");
             cpu.state.fpr[4] = 0x7FF8000000000001ull;
             step(cpu, xtype(63, 32, 12, 4, 5));
             expect(cpu.state.getCRField(3), 1, "unordered comparison");
             expect(cpu.state.fpscr & 0x00080000, 0x80000,
                    "ordered NaN exception");
         }});
    tests.push_back({"FSEL", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(-0.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(63, 23, 3, 4, 5) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 3.0,
                                     "select with negative zero");
                     }});
    tests.push_back({"MFFS", [](Broadway &cpu) {
                         cpu.state.fpscr = 0x12345678;
                         step(cpu, xtype(63, 583, 3, 0, 0));
                         expect(static_cast<uint32_t>(cpu.state.fpr[3]),
                                0x12345678, "FPSCR read");
                     }});
    tests.push_back({"MTFSF", [](Broadway &cpu) {
                         cpu.state.fpr[5] = 0x12345002;
                         step(cpu,
                              (63u << 26) | (1 << 17) | (5 << 11) | (711 << 1));
                         expect(cpu.state.fpscr, 2, "selected FPSCR field");
                     }});
    tests.push_back({"MTFSFI", [](Broadway &cpu) {
                         step(cpu,
                              (63u << 26) | (7 << 23) | (3 << 12) | (134 << 1));
                         expect(cpu.state.fpscr, 3, "FPSCR immediate field");
                     }});
    tests.push_back({"MTFSB0", [](Broadway &cpu) {
                         cpu.state.fpscr = 1;
                         step(cpu, xtype(63, 70, 31, 0, 0));
                         expect(cpu.state.fpscr, 0, "FPSCR bit");
                     }});
    tests.push_back({"MTFSB1", [](Broadway &cpu) {
                         cpu.state.fpscr = 0;
                         step(cpu, xtype(63, 38, 31, 0, 0));
                         expect(cpu.state.fpscr, 1, "FPSCR bit");
                     }});
    tests.push_back({"MCRFS", [](Broadway &cpu) {
                         cpu.state.fpscr = 0x80000000;
                         step(cpu, xtype(63, 64, 12, 0, 0));
                         expect(cpu.state.getCRField(3), 8,
                                "FPSCR field to CR");
                         expect(cpu.state.fpscr, 0, "exception flag cleared");
                     }});
    tests.push_back({"PS_ADD", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 21, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 10.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 18.0, "PS1 result");
                     }});
    tests.push_back({"PS_SUB", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 20, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 6.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 14.0, "PS1 result");
                     }});
    tests.push_back({"PS_DIV", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 18, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 4.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 8.0, "PS1 result");
                     }});
    tests.push_back({"PS_MUL", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 25, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 24.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 64.0, "PS1 result");
                     }});
    tests.push_back({"PS_MADD", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 29, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 26.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 66.0, "PS1 result");
                     }});
    tests.push_back({"PS_MSUB", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 28, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 22.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 62.0, "PS1 result");
                     }});
    tests.push_back({"PS_NMADD", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 31, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], -26.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], -66.0, "PS1 result");
                     }});
    tests.push_back({"PS_NMSUB", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 30, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], -22.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], -62.0, "PS1 result");
                     }});
    tests.push_back({"PS_SUM0", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 10, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 10.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 4.0, "PS1 result");
                     }});
    tests.push_back({"PS_SUM1", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 11, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 3.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 10.0, "PS1 result");
                     }});
    tests.push_back({"PS_MULS0", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 12, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 24.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 48.0, "PS1 result");
                     }});
    tests.push_back({"PS_MULS1", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 13, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 32.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 64.0, "PS1 result");
                     }});
    tests.push_back({"PS_MADDS0", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 14, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 26.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 50.0, "PS1 result");
                     }});
    tests.push_back({"PS_MADDS1", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 15, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 34.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 66.0, "PS1 result");
                     }});
    tests.push_back({"PS_SEL", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(8.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(16.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[6] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[6] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 23, 3, 4, 5, true) | (6 << 6));
                         expectFloat(cpu.state.fpr[3], 3.0, "PS0 result");
                         expectFloat(cpu.state.ps1[3], 4.0, "PS1 result");
                     }});
    tests.push_back(
        {"PS_RES", [](Broadway &cpu) {
             cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
             cpu.state.ps1[5] = std::bit_cast<uint64_t>(16.0);
             step(cpu, xtype(4, 24, 3, 0, 5, true));
             expect(cpu.state.fpr[3], 0x3FCFFF0000000000ull, "PS0 unary");
             expect(cpu.state.ps1[3], 0x3FAFFF0000000000ull, "PS1 unary");
         }});
    tests.push_back(
        {"PS_RSQRTE", [](Broadway &cpu) {
             cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
             cpu.state.ps1[5] = std::bit_cast<uint64_t>(16.0);
             step(cpu, xtype(4, 26, 3, 0, 5, true));
             expect(cpu.state.fpr[3], 0x3FDFFE8000000000ull, "PS0 unary");
             expect(cpu.state.ps1[3], 0x3FCFFE8000000000ull, "PS1 unary");
         }});
    tests.push_back({"PS_NEG", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(16.0);
                         step(cpu, xtype(4, 40, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], -4.0, "PS0 unary");
                         expectFloat(cpu.state.ps1[3], -16.0, "PS1 unary");
                     }});
    tests.push_back({"PS_ABS", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(16.0);
                         step(cpu, xtype(4, 264, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], 4.0, "PS0 unary");
                         expectFloat(cpu.state.ps1[3], 16.0, "PS1 unary");
                     }});
    tests.push_back({"PS_NABS", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(16.0);
                         step(cpu, xtype(4, 136, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], -4.0, "PS0 unary");
                         expectFloat(cpu.state.ps1[3], -16.0, "PS1 unary");
                     }});
    tests.push_back({"PS_MR", [](Broadway &cpu) {
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(16.0);
                         step(cpu, xtype(4, 72, 3, 0, 5, true));
                         expectFloat(cpu.state.fpr[3], 4.0, "PS0 unary");
                         expectFloat(cpu.state.ps1[3], 16.0, "PS1 unary");
                     }});
    tests.push_back({"PS_MERGE00", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 528, 4, 4, 5));
                         expectFloat(cpu.state.fpr[4], 1.0, "PS0 alias merge");
                         expectFloat(cpu.state.ps1[4], 3.0, "PS1 alias merge");
                     }});
    tests.push_back({"PS_MERGE01", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 560, 4, 4, 5));
                         expectFloat(cpu.state.fpr[4], 1.0, "PS0 alias merge");
                         expectFloat(cpu.state.ps1[4], 4.0, "PS1 alias merge");
                     }});
    tests.push_back({"PS_MERGE10", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 592, 4, 4, 5));
                         expectFloat(cpu.state.fpr[4], 2.0, "PS0 alias merge");
                         expectFloat(cpu.state.ps1[4], 3.0, "PS1 alias merge");
                     }});
    tests.push_back({"PS_MERGE11", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(3.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(4.0);
                         step(cpu, xtype(4, 624, 4, 4, 5));
                         expectFloat(cpu.state.fpr[4], 2.0, "PS0 alias merge");
                         expectFloat(cpu.state.ps1[4], 4.0, "PS1 alias merge");
                     }});
    tests.push_back({"PS_CMPU0", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(4, 0, 12, 4, 5));
                         expect(cpu.state.getCRField(3), 8,
                                "paired comparison lane");
                     }});
    tests.push_back({"PS_CMPO0", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(4, 32, 12, 4, 5));
                         expect(cpu.state.getCRField(3), 8,
                                "paired comparison lane");
                     }});
    tests.push_back({"PS_CMPU1", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(4, 64, 12, 4, 5));
                         expect(cpu.state.getCRField(3), 4,
                                "paired comparison lane");
                     }});
    tests.push_back({"PS_CMPO1", [](Broadway &cpu) {
                         cpu.state.fpr[4] = std::bit_cast<uint64_t>(1.0);
                         cpu.state.ps1[4] = std::bit_cast<uint64_t>(4.0);
                         cpu.state.fpr[5] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[5] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(4, 96, 12, 4, 5));
                         expect(cpu.state.getCRField(3), 4,
                                "paired comparison lane");
                     }});
    tests.push_back(
        {"PSQ_L", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA - 8;
             cpu.state.gpr[5] = 8;
             cpu.state.spr[SPR::GQR1] = 0x02040204;
             Bus::write8(DATA, 8);
             Bus::write8(DATA + 1, 12);
             step(cpu, (56u << 26) | (3 << 21) | (4 << 16) | (1 << 12) | 8);
             expectFloat(cpu.state.fpr[3], 2.0, "dequantized PS0");
             expectFloat(cpu.state.ps1[3], 3.0, "dequantized PS1");
             expect(cpu.state.gpr[4], DATA - 8, "quantized base update");

             cpu.state.gpr[4] = DATA;
             cpu.state.spr[SPR::GQR1] = 0x01070000;
             Bus::write16(DATA, 0xFFF8);
             step(cpu,
                  (56u << 26) | (3 << 21) | (4 << 16) | (1 << 15) | (1 << 12));
             expectFloat(cpu.state.fpr[3], -4.0,
                         "signed quantization and scaling");
             expectFloat(cpu.state.ps1[3], 1.0,
                         "W supplies one in second lane");
             cpu.state.spr[SPR::GQR1] = 0x3F050000;
             Bus::write16(DATA, 32769);
             step(cpu,
                  (56u << 26) | (3 << 21) | (4 << 16) | (1 << 15) | (1 << 12));
             expectFloat(cpu.state.fpr[3], 65538.0,
                         "unsigned halfword and negative scale");
             cpu.state.spr[SPR::GQR1] = 0x00060000;
             Bus::write8(DATA, 0x80);
             step(cpu,
                  (56u << 26) | (3 << 21) | (4 << 16) | (1 << 15) | (1 << 12));
             expectFloat(cpu.state.fpr[3], -128.0, "signed byte");
             cpu.state.spr[SPR::GQR1] = 0;
             Bus::write32(DATA, 0x3FC00000);
             step(cpu,
                  (56u << 26) | (3 << 21) | (4 << 16) | (1 << 15) | (1 << 12));
             expectFloat(cpu.state.fpr[3], 1.5, "unquantized float");
         }});
    tests.push_back(
        {"PSQ_LU", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA - 8;
             cpu.state.gpr[5] = 8;
             cpu.state.spr[SPR::GQR1] = 0x02040204;
             Bus::write8(DATA, 8);
             Bus::write8(DATA + 1, 12);
             step(cpu, (57u << 26) | (3 << 21) | (4 << 16) | (1 << 12) | 8);
             expectFloat(cpu.state.fpr[3], 2.0, "dequantized PS0");
             expectFloat(cpu.state.ps1[3], 3.0, "dequantized PS1");
             expect(cpu.state.gpr[4], DATA, "quantized base update");
         }});
    tests.push_back(
        {"PSQ_ST", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA - 8;
             cpu.state.gpr[5] = 8;
             cpu.state.spr[SPR::GQR1] = 0x02040204;
             cpu.state.fpr[3] = std::bit_cast<uint64_t>(2.0);
             cpu.state.ps1[3] = std::bit_cast<uint64_t>(3.0);
             step(cpu, (60u << 26) | (3 << 21) | (4 << 16) | (1 << 12) | 8);
             expect(Bus::read8(DATA), 8, "quantized PS0");
             expect(Bus::read8(DATA + 1), 12, "quantized PS1");
             expect(cpu.state.gpr[4], DATA - 8, "quantized base update");

             cpu.state.gpr[4] = DATA;
             cpu.state.spr[SPR::GQR1] = 7;
             cpu.state.fpr[3] = std::bit_cast<uint64_t>(40000.0);
             cpu.state.ps1[3] = std::bit_cast<uint64_t>(-40000.0);
             step(cpu, (60u << 26) | (3 << 21) | (4 << 16) | (1 << 12));
             expect(Bus::read16(DATA), 0x7FFF, "positive saturation");
             expect(Bus::read16(DATA + 2), 0x8000, "negative saturation");
             cpu.state.spr[SPR::GQR1] = 0;
             cpu.state.fpr[3] = std::bit_cast<uint64_t>(1.5);
             Bus::write32(DATA + 4, 0x12345678);
             step(cpu,
                  (60u << 26) | (3 << 21) | (4 << 16) | (1 << 15) | (1 << 12));
             expect(Bus::read32(DATA), 0x3FC00000, "unquantized store");
             expect(Bus::read32(DATA + 4), 0x12345678,
                    "W preserves second element");
         }});
    tests.push_back(
        {"PSQ_STU", [](Broadway &cpu) {
             cpu.state.gpr[4] = DATA - 8;
             cpu.state.gpr[5] = 8;
             cpu.state.spr[SPR::GQR1] = 0x02040204;
             cpu.state.fpr[3] = std::bit_cast<uint64_t>(2.0);
             cpu.state.ps1[3] = std::bit_cast<uint64_t>(3.0);
             step(cpu, (61u << 26) | (3 << 21) | (4 << 16) | (1 << 12) | 8);
             expect(Bus::read8(DATA), 8, "quantized PS0");
             expect(Bus::read8(DATA + 1), 12, "quantized PS1");
             expect(cpu.state.gpr[4], DATA, "quantized base update");
         }});
    tests.push_back({"PSQ_LX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.spr[SPR::GQR1] = 0x02040204;
                         Bus::write8(DATA, 8);
                         Bus::write8(DATA + 1, 12);
                         step(cpu, xtype(4, 6) | (1 << 7));
                         expectFloat(cpu.state.fpr[3], 2.0, "dequantized PS0");
                         expectFloat(cpu.state.ps1[3], 3.0, "dequantized PS1");
                         expect(cpu.state.gpr[4], DATA - 8,
                                "quantized base update");
                     }});
    tests.push_back({"PSQ_LUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.spr[SPR::GQR1] = 0x02040204;
                         Bus::write8(DATA, 8);
                         Bus::write8(DATA + 1, 12);
                         step(cpu, xtype(4, 38) | (1 << 7));
                         expectFloat(cpu.state.fpr[3], 2.0, "dequantized PS0");
                         expectFloat(cpu.state.ps1[3], 3.0, "dequantized PS1");
                         expect(cpu.state.gpr[4], DATA,
                                "quantized base update");
                     }});
    tests.push_back({"PSQ_STX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.spr[SPR::GQR1] = 0x02040204;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(4, 7) | (1 << 7));
                         expect(Bus::read8(DATA), 8, "quantized PS0");
                         expect(Bus::read8(DATA + 1), 12, "quantized PS1");
                         expect(cpu.state.gpr[4], DATA - 8,
                                "quantized base update");
                     }});
    tests.push_back({"PSQ_STUX", [](Broadway &cpu) {
                         cpu.state.gpr[4] = DATA - 8;
                         cpu.state.gpr[5] = 8;
                         cpu.state.spr[SPR::GQR1] = 0x02040204;
                         cpu.state.fpr[3] = std::bit_cast<uint64_t>(2.0);
                         cpu.state.ps1[3] = std::bit_cast<uint64_t>(3.0);
                         step(cpu, xtype(4, 39) | (1 << 7));
                         expect(Bus::read8(DATA), 8, "quantized PS0");
                         expect(Bus::read8(DATA + 1), 12, "quantized PS1");
                         expect(cpu.state.gpr[4], DATA,
                                "quantized base update");
                     }});
    size_t passed = 0;
    for (const auto &test : tests) {
        device->cpu.reset(CODE);
        device->cpu.state.msr = 0x2000;
        device->cpu.state.spr[SPR::HID2] = 0xA0000000;
        for (uint32_t i = 0; i < 256; ++i)
            Bus::write8(DATA + i, 0);
        std::cout << std::left << std::setw(20) << test.name << " ..... ";
        try {
            test.run(device->cpu);
            ++passed;
            std::cout << "\033[32m[PASSED]\033[0m\n";
        } catch (const std::exception &error) {
            std::cout << "\033[31m[FAILED]\033[0m " << error.what() << '\n';
        }
    }
    std::cout << '\n'
              << passed << "/" << tests.size() << " Broadway tests passed.\n";
    return passed == tests.size() ? 0 : 1;
}
