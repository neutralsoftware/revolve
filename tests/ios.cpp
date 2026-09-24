#include "device.h"
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void expect(uint32_t actual, uint32_t expected, const std::string &field) {
    if (actual != expected)
        throw std::runtime_error(field + ": expected " +
                                 utils::toHexString(expected) + ", got " +
                                 utils::toHexString(actual));
}

void expect(bool actual, bool expected, const std::string &field) {
    if (actual != expected)
        throw std::runtime_error(
            field + ": expected " + std::string(expected ? "true" : "false") +
            ", got " + std::string(actual ? "true" : "false"));
}

struct IOSTest {
    const char *name;
    std::function<void(Device &)> run;
};
}

int runIOSSuite() {
    std::vector<IOSTest> tests;

    tests.push_back(
        {"PPC_TO_STARLET", [](Device &device) {
             constexpr uint32_t requestAddress = 0x00001000;

             device.ipc->write(IPC_PPCMSG, requestAddress, AccessSize::U32);
             device.ipc->write(IPC_PPCCTRL, 1u << 0, AccessSize::U32);

             expect(device.ipc->ppcRequestPending(), true, "request pending");
             expect(device.ipc->getPPCMessage(), requestAddress,
                    "request address");
             expect(device.ipc->read(IPC_PPCCTRL, AccessSize::U32) & (1u << 0),
                    1u << 0, "X1 request bit");

             device.ipc->acknoledgeFromStarlet();

             expect(device.ipc->ppcRequestPending(), false, "request consumed");
             expect(device.ipc->read(IPC_PPCCTRL, AccessSize::U32) &
                        ((1u << 0) | (1u << 1)),
                    1u << 1, "Starlet acknowledgement");
         }});

    tests.push_back(
        {"STARLET_TO_PPC", [](Device &device) {
             constexpr uint32_t responseAddress = 0x00002000;

             device.controller.setMask(
                 1u << static_cast<uint32_t>(HollywoodIRQ::IPC));
             device.pi->write(
                 0x04, 1u << static_cast<uint32_t>(PIInterrupt::Hollywood),
                 AccessSize::U32);
             device.ipc->write(IPC_PPCCTRL, 1u << 4, AccessSize::U32);

             device.ipc->replyFromStarlet(responseAddress);

             expect(device.ipc->read(IPC_ARMMSG, AccessSize::U32),
                    responseAddress, "response address");
             expect(device.ipc->read(IPC_PPCCTRL, AccessSize::U32) &
                        ((1u << 2) | (1u << 4)),
                    (1u << 2) | (1u << 4), "Y1 response state");
             expect(device.pi->interruptPending(), true, "PPC interrupt");

             device.ipc->write(IPC_PPCCTRL, (1u << 2) | (1u << 4),
                               AccessSize::U32);

             expect(device.ipc->read(IPC_PPCCTRL, AccessSize::U32) & (1u << 2),
                    0, "response acknowledged");
             expect(device.pi->interruptPending(), true,
                    "Hollywood interrupt remains latched");
             Bus::writePhysical32(IPC_MMIO_BASE + 0x30, 1u << 30);
             expect(device.pi->interruptPending(), false,
                    "PPC interrupt cleared");
         }});

    tests.push_back(
        {"DSP_CONTROL", [](Device &device) {
             device.dsp->write(0x0A, 0x08AD, AccessSize::U16);
             const uint32_t control =
                 device.dsp->read(0x0A, AccessSize::U16);
             expect(control & 1, 0, "DSP reset self clear");
             expect(control & 4, 4, "DSP halt state");
             expect(control & 0x0C00, 0x0800, "DSP initialization state");
             expect(device.dsp->read(0x04, AccessSize::U16), 0,
                    "DSP initialization waits while halted");
             device.dsp->write(0x0A, 0, AccessSize::U16);
             expect(device.dsp->read(0x04, AccessSize::U16), 0x8054,
                    "DSP initialization mailbox high");
             expect(device.dsp->read(0x06, AccessSize::U16), 0x4348,
                    "DSP initialization mailbox low");
             expect(device.dsp->read(0x04, AccessSize::U16), 0x0054,
                    "DSP initialization mailbox consumed");
             expect(device.dsp->read(0x16, AccessSize::U16), 1,
                    "ARAM initialized");
             for (uint32_t offset = 0; offset < 32; ++offset)
                 Bus::writePhysical8(0x1000 + offset,
                                     static_cast<uint8_t>(offset + 1));
             device.dsp->write(0x20, 0, AccessSize::U16);
             device.dsp->write(0x22, 0x1000, AccessSize::U16);
             device.dsp->write(0x24, 0, AccessSize::U16);
             device.dsp->write(0x26, 0, AccessSize::U16);
             device.dsp->write(0x28, 0, AccessSize::U16);
             device.dsp->write(0x2A, 32, AccessSize::U16);
             expect(device.dsp->read(0x0A, AccessSize::U16) & (1u << 5),
                    1u << 5, "ARAM completion flag");
             for (uint32_t offset = 0; offset < 32; ++offset)
                 expect(Bus::readPhysical8(0x10000000 + offset), offset + 1,
                        "ARAM copy");
             device.dsp->write(0x0A, 1u << 5, AccessSize::U16);
             expect(device.dsp->read(0x0A, AccessSize::U16) & (1u << 5), 0,
                    "ARAM completion acknowledged");
         }});

    tests.push_back(
        {"EXI_TRANSFER", [](Device &device) {
             device.pi->write(0x04,
                              1u << static_cast<uint32_t>(PIInterrupt::EXI),
                              AccessSize::U32);
             device.exi->write(0x00, 1u << 2, AccessSize::U32);
             device.exi->write(0x0C, 1, AccessSize::U32);
             expect(device.exi->read(0x0C, AccessSize::U32) & 1, 0,
                    "EXI transfer completion");
             expect(device.exi->read(0x00, AccessSize::U32) & 8, 8,
                    "EXI completion flag");
             expect(device.pi->interruptPending(), true, "EXI interrupt");
             device.exi->write(0x00, (1u << 2) | (1u << 3),
                               AccessSize::U32);
             expect(device.pi->interruptPending(), false,
                    "EXI interrupt acknowledged");
         }});

    tests.push_back(
        {"SI_TRANSFER", [](Device &device) {
             device.pi->write(0x04,
                              1u << static_cast<uint32_t>(PIInterrupt::SI),
                              AccessSize::U32);
             device.si->write(0x34, (1u << 30) | 1, AccessSize::U32);
             const uint32_t control = device.si->read(0x34, AccessSize::U32);
             expect(control & 1, 0, "SI transfer completion");
             expect(control & (1u << 31), 1u << 31,
                    "SI completion flag");
             expect(control & (1u << 29), 1u << 29,
                    "SI absent device error");
             expect(device.si->read(0x38, AccessSize::U32) & (1u << 27),
                    1u << 27, "SI no response status");
             expect(device.pi->interruptPending(), true, "SI interrupt");
             device.si->write(0x34, (1u << 31) | (1u << 30),
                              AccessSize::U32);
             expect(device.pi->interruptPending(), false,
                    "SI interrupt acknowledged");
         }});

    tests.push_back(
        {"HOLLYWOOD_COMPAT", [](Device &device) {
             expect(device.ipc->read(0x64, AccessSize::U32), 0xFFFFFFFF,
                    "AHB protection");
             device.ipc->write(0xC0, 0xFFFFFFFF, AccessSize::U32);
             expect(device.ipc->read(0xC0, AccessSize::U32), 0xC3A0,
                    "Broadway GPIO output ownership");
             device.ipc->write(0xE0, 0x12345678, AccessSize::U32);
             expect(device.ipc->read(0xE0, AccessSize::U32), 0x1234D7F8,
                    "Starlet GPIO output ownership");
             device.ipc->write(0xC4, 0, AccessSize::U32);
             expect(device.ipc->read(0xC4, AccessSize::U32), 0x00FF1C1F,
                    "Broadway GPIO direction ownership");
             expect(device.ipc->read(0xC8, AccessSize::U32), 0,
                    "GPIO disc input");
             device.ipc->write(0x194, 0xFFFFFBFF, AccessSize::U32);
             expect(device.ipc->read(0x194, AccessSize::U32), 0xFFFFFBFF,
                    "hardware reset latch");
             for (uint32_t offset : {0x180u, 0x1CCu, 0x1D0u}) {
                 device.ipc->write(offset, 0xFFFFFFFF, AccessSize::U32);
                 expect(device.ipc->read(offset, AccessSize::U32), 0,
                        "Hollywood compatibility register");
             }
         }});

    size_t passed = 0;
    for (const auto &test : tests) {
        auto device = Device::createDevice();
        std::cout << std::left << std::setw(20) << test.name << " ..... ";
        try {
            test.run(*device);
            ++passed;
            std::cout << "\033[32m[PASSED]\033[0m\n";
        } catch (const std::exception &error) {
            std::cout << "\033[31m[FAILED]\033[0m " << error.what() << '\n';
        }
    }

    std::cout << '\n'
              << passed << "/" << tests.size() << " IOS tests passed.\n";
    return passed == tests.size() ? 0 : 1;
}
