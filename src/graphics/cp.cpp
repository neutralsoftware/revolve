#include "graphics/cp.h"
#include "core/utils.h"

uint32_t CommandProcessor::read(uint32_t offset, AccessSize size) {
    if (size != AccessSize::U16) {
        Logger::log("CP", LogLevel::Warning,
                    "Unsupported read size at " + utils::toHexString(offset));

        return 0;
    }

    return read16(offset);
}

void CommandProcessor::write(uint32_t offset, uint32_t value, AccessSize size) {
    if (size != AccessSize::U16) {
        Logger::log("CP", LogLevel::Warning,
                    "Unsupported write size at " + utils::toHexString(offset));

        return;
    }

    write16(offset, static_cast<uint16_t>(value));
}

uint16_t CommandProcessor::read16(uint32_t offset) const {
    switch (static_cast<CPRegister>(offset)) {
    case CPRegister::Status: {
        uint16_t value = 0;

        if (state.bpInterrupt)
            value |= 1 << 4;

        if (state.commandIdle)
            value |= 1 << 3;

        if (state.readIdle)
            value |= 1 << 2;

        if (state.underflow)
            value |= 1 << 1;

        if (state.overflow)
            value |= 1 << 0;

        return value;
    }

    case CPRegister::Control: {
        uint16_t value = 0;

        if (state.breakpointEnable)
            value |= 1 << 5;

        if (state.fifoLinkEnable)
            value |= 1 << 4;

        if (state.underflowInterruptEnable)
            value |= 1 << 3;

        if (state.overflowInterruptEnable)
            value |= 1 << 2;

        if (state.cpInterruptEnable)
            value |= 1 << 1;

        if (state.fifoReadEnable)
            value |= 1 << 0;

        return value;
    }

    case CPRegister::Token:
        return state.token;

    case CPRegister::FifoStartHi:
        return state.fifoStart >> 16;

    case CPRegister::FifoStartLo:
        return state.fifoStart & 0xFFFF;

    case CPRegister::FifoEndHi:
        return state.fifoEnd >> 16;

    case CPRegister::FifoEndLo:
        return state.fifoEnd & 0xFFFF;

    case CPRegister::FifoWritePointerHi:
        return state.fifoWritePointer >> 16;

    case CPRegister::FifoWritePointerLo:
        return state.fifoWritePointer & 0xFFFF;

    default:
        Logger::log("CP", LogLevel::Warning,
                    "Unknown register read: " + utils::toHexString(offset));

        return 0;
    }
}

void CommandProcessor::write16(uint32_t offset, uint16_t value) {
    switch (static_cast<CPRegister>(offset)) {
    case CPRegister::Control:
        state.breakpointEnable = value & (1 << 5);

        state.fifoLinkEnable = value & (1 << 4);

        state.underflowInterruptEnable = value & (1 << 3);

        state.overflowInterruptEnable = value & (1 << 2);

        state.cpInterruptEnable = value & (1 << 1);

        state.fifoReadEnable = value & (1 << 0);

        return;

    case CPRegister::Clear:
        if (value & (1 << 1))
            state.underflow = false;

        if (value & (1 << 0))
            state.overflow = false;

        return;

    case CPRegister::FifoStartHi:
        state.fifoStart = (state.fifoStart & 0x0000FFFF) |
                          (static_cast<uint32_t>(value) << 16);
        return;

    case CPRegister::FifoStartLo:
        state.fifoStart = (state.fifoStart & 0xFFFF0000) | value;
        return;

    case CPRegister::FifoEndHi:
        state.fifoEnd =
            (state.fifoEnd & 0x0000FFFF) | (static_cast<uint32_t>(value) << 16);
        return;

    case CPRegister::FifoEndLo:
        state.fifoEnd = (state.fifoEnd & 0xFFFF0000) | value;
        return;

    case CPRegister::FifoWritePointerHi:
        state.fifoWritePointer = (state.fifoWritePointer & 0x0000FFFF) |
                                 (static_cast<uint32_t>(value) << 16);
        return;

    case CPRegister::FifoWritePointerLo:
        state.fifoWritePointer = (state.fifoWritePointer & 0xFFFF0000) | value;
        return;

    default:
        Logger::log("CP", LogLevel::Warning,
                    "Unknown register write: " + utils::toHexString(offset));
        return;
    }
}