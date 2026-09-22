#include "graphics/video_interface.h"
#include "core/memory.h"
#include "core/utils.h"
#include "cpu/interface.h"
#include "device.h"
#include <cstdint>

uint32_t VideoInterface::read(uint32_t offset, AccessSize size) {
    if (size == AccessSize::U16) {
        switch (static_cast<VIRegister>(offset)) {
        case VIRegister::VerticalBeam:
            return state.currentVerticalPosition;

        case VIRegister::HorizontalBeam:
            return state.currentHorizontalPosition;

        default:
            return registers[offset / 2];
        }
    }

    if (size == AccessSize::U32) {
        uint32_t hi = read(offset, AccessSize::U16);
        uint32_t lo = read(offset + 2, AccessSize::U16);

        return (hi << 16) | lo;
    }

    return 0;
}

void VideoInterface::write(uint32_t offset, uint32_t value, AccessSize size) {
    switch (size) {
    case AccessSize::U16:
        registers[offset / 2] = static_cast<uint16_t>(value);
        return;

    case AccessSize::U32:
        registers[offset / 2] = static_cast<uint16_t>(value >> 16);

        registers[offset / 2 + 1] = static_cast<uint16_t>(value);

        break;

    default:
        Logger::log("VI", LogLevel::Error, "Invalid access size");
        return;
    }

    decodeRegisterWrite(offset, value, size);
}

void VideoInterface::decodeRegisterWrite(uint32_t offset, uint32_t value,
                                         AccessSize size) {
    switch (static_cast<VIRegister>(offset)) {
    case VIRegister::VerticalTiming:
        state.verticalTiming =
            registers[static_cast<uint32_t>(VIRegister::VerticalTiming) / 2];
        break;
    case VIRegister::Control:
        state.displayConfig =
            registers[static_cast<uint32_t>(VIRegister::Control) / 2];
        break;
    case VIRegister::HorizontalTiming0Hi:
    case VIRegister::HorizontalTiming0Lo:
        state.horizontalTiming0 = readRegister32(
            static_cast<uint32_t>(VIRegister::HorizontalTiming0Hi));
        break;
    case VIRegister::HorizontalTiming1Hi:
    case VIRegister::HorizontalTiming1Lo:
        state.horizontalTiming1 = readRegister32(
            static_cast<uint32_t>(VIRegister::HorizontalTiming1Hi));
        break;
    case VIRegister::FbTopHi:
    case VIRegister::FbTopLo:
        state.topFramebuffer =
            readRegister32(static_cast<uint32_t>(VIRegister::FbTopHi));
        break;
    case VIRegister::FbBottomHi:
    case VIRegister::FbBottomLo:
        state.bottomFramebuffer =
            readRegister32(static_cast<uint32_t>(VIRegister::FbBottomHi));
        break;
    case VIRegister::Interupt0Hi:
    case VIRegister::Interupt0Lo:
        state.displayInterrupts[0] =
            readRegister32(static_cast<uint32_t>(VIRegister::Interupt0Hi));
        break;
    case VIRegister::Interupt1Hi:
    case VIRegister::Interupt1Lo:
        state.displayInterrupts[1] =
            readRegister32(static_cast<uint32_t>(VIRegister::Interupt1Hi));
        break;
    case VIRegister::Interupt2Hi:
    case VIRegister::Interupt2Lo:
        state.displayInterrupts[2] =
            readRegister32(static_cast<uint32_t>(VIRegister::Interupt2Hi));
        break;
    case VIRegister::Interupt3Hi:
    case VIRegister::Interupt3Lo:
        state.displayInterrupts[3] =
            readRegister32(static_cast<uint32_t>(VIRegister::Interupt3Hi));
        break;
    default:
        Logger::log(
            "VI", LogLevel::Error,
            "Write to unknown register offset: 0x" + std::to_string(offset) +
                " with value: 0x" + std::to_string(value) +
                " and size: " + std::to_string(static_cast<uint32_t>(size)));
        break;
    }
}

void VideoInterface::initialize() {
    Device::globalDevice->scheduler.schedule("VI Cycles", onScanLine,
                                             VI_CYCLES_PER_LINE);
}

void VideoInterface::onScanLine() {
    state.currentVerticalPosition++;

    if (state.currentVerticalPosition > VI_TOTAL_LINES) {
        state.currentVerticalPosition = 1;
    }

    checkInterrupts();
}

void VideoInterface::checkInterrupts() {
    bool irq = false;

    for (uint32_t reg : state.displayInterrupts) {
        uint32_t targetVertical = reg >> 16;
        bool enabled = (reg & 0x8000) != 0;

        if (enabled && state.currentVerticalPosition == targetVertical) {
            irq = true;
            break;
        }
    }

    if (irq)
        Device::globalDevice->pi->raiseInterrupt(PIInterrupt::VI);
    else {
        Device::globalDevice->pi->clearInterrupt(PIInterrupt::VI);
    }
}