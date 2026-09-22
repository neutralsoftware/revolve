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
    const auto isInterruptHigh = [](uint32_t off) {
        return off == static_cast<uint32_t>(VIRegister::Interupt0Hi) ||
               off == static_cast<uint32_t>(VIRegister::Interupt1Hi) ||
               off == static_cast<uint32_t>(VIRegister::Interupt2Hi) ||
               off == static_cast<uint32_t>(VIRegister::Interupt3Hi);
    };

    switch (size) {
    case AccessSize::U16: {
        uint16_t newValue = static_cast<uint16_t>(value);

        if (isInterruptHigh(offset)) {
            constexpr uint16_t STATUS = 0x8000;

            uint16_t oldValue = registers[offset / 2];

            if (newValue & STATUS)
                newValue = (newValue & ~STATUS) | (oldValue & STATUS);
            else
                newValue &= ~STATUS;
        }

        registers[offset / 2] = newValue;
        break;
    }

    case AccessSize::U32: {
        uint32_t newValue = value;

        if (isInterruptHigh(offset)) {
            constexpr uint32_t STATUS = 1u << 31;

            uint32_t oldValue = readRegister32(offset);

            if (newValue & STATUS)
                newValue = (newValue & ~STATUS) | (oldValue & STATUS);
            else
                newValue &= ~STATUS;
        }

        writeRegister32(offset, newValue);
        break;
    }

    default:
        Logger::log("VI", LogLevel::Error, "Invalid access size");
        return;
    }

    decodeRegisterWrite(offset, value, size);
    checkInterrupts();
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
    case VIRegister::Interupt1Hi:
    case VIRegister::Interupt1Lo:
    case VIRegister::Interupt2Hi:
    case VIRegister::Interupt2Lo:
    case VIRegister::Interupt3Hi:
    case VIRegister::Interupt3Lo:
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
    Device::globalDevice->scheduler.schedule(
        "VI Cycles", [this]() { onScanLine(); }, VI_CYCLES_PER_LINE);
}

void VideoInterface::onScanLine() {
    state.currentVerticalPosition++;

    if (state.currentVerticalPosition > VI_TOTAL_LINES) {
        state.currentVerticalPosition = 1;
        onFrame();
    }

    checkInterrupts();

    Device::globalDevice->scheduler.schedule(
        "VI Cycles", [this]() { onScanLine(); }, VI_CYCLES_PER_LINE);
}

constexpr uint32_t VI_DI_STATUS = 1u << 31;
constexpr uint32_t VI_DI_ENABLE = 1u << 28;

constexpr uint32_t VI_DI_VCT_MASK = 0x03FF0000;
constexpr uint32_t VI_DI_HCT_MASK = 0x000003FF;

constexpr uint32_t VI_DI_VCT_SHIFT = 16;

void VideoInterface::checkInterrupts() {
    bool shouldRaise = false;

    for (size_t i = 0; i < 4; ++i) {
        uint32_t base = static_cast<uint32_t>(VIRegister::Interupt0Hi) +
                        static_cast<uint32_t>(i) * 4;

        uint32_t reg = readRegister32(base);

        const uint32_t targetVertical =
            (reg & VI_DI_VCT_MASK) >> VI_DI_VCT_SHIFT;

        const bool enabled = (reg & VI_DI_ENABLE) != 0;

        const bool pending = (reg & VI_DI_STATUS) != 0;

        if (!pending && state.currentVerticalPosition == targetVertical) {
            reg |= VI_DI_STATUS;
            writeRegister32(base, reg);
        }

        if ((reg & VI_DI_ENABLE) && (reg & VI_DI_STATUS)) {
            shouldRaise = true;
        }
    }

    if (shouldRaise)
        Device::globalDevice->pi->raiseInterrupt(PIInterrupt::VI);
    else
        Device::globalDevice->pi->clearInterrupt(PIInterrupt::VI);
}

void VideoInterface::onFrame() {
    auto &renderer = Device::globalDevice->gx.renderer;

    renderer->endFrame();
    renderer->beginFrame();
}

uint32_t VideoInterface::readRegister32(uint32_t offset) const {
    uint16_t hi = registers[offset / 2];
    uint16_t lo = registers[offset / 2 + 1];

    return (static_cast<uint32_t>(hi) << 16) | static_cast<uint32_t>(lo);
}

void VideoInterface::writeRegister32(uint32_t offset, uint32_t value) {
    uint16_t hi = static_cast<uint16_t>(value >> 16);
    uint16_t lo = static_cast<uint16_t>(value & 0xFFFF);

    registers[offset / 2] = hi;
    registers[offset / 2 + 1] = lo;
}