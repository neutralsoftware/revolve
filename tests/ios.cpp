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
             expect(device.pi->interruptPending(), false,
                    "PPC interrupt cleared");
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
