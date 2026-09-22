#include "graphics/cp.h"
#include "core/utils.h"
#include "cpu/interface.h"
#include "device.h"

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
        return state.fifo.base >> 16;

    case CPRegister::FifoStartLo:
        return state.fifo.base & 0xFFFF;

    case CPRegister::FifoEndHi:
        return state.fifo.end >> 16;

    case CPRegister::FifoEndLo:
        return state.fifo.end & 0xFFFF;

    case CPRegister::FifoWritePointerHi:
        return state.fifo.writePointer >> 16;

    case CPRegister::FifoWritePointerLo:
        return state.fifo.writePointer & 0xFFFF;

    case CPRegister::FifoHighWatermarkHi:
        return state.fifo.highWatermark >> 16;

    case CPRegister::FifoHighWatermarkLo:
        return state.fifo.highWatermark & 0xFFFF;

    case CPRegister::FifoLowWatermarkHi:
        return state.fifo.lowWatermark >> 16;

    case CPRegister::FifoLowWatermarkLo:
        return state.fifo.lowWatermark & 0xFFFF;

    case CPRegister::FifoReadWriteDistanceHi:
        return state.fifo.readWriteDistance >> 16;

    case CPRegister::FifoReadWriteDistanceLo:
        return state.fifo.readWriteDistance & 0xFFFF;

    case CPRegister::FifoReadPointerHi:
        return state.fifo.readPointer >> 16;

    case CPRegister::FifoReadPointerLo:
        return state.fifo.readPointer & 0xFFFF;

    case CPRegister::FifoBreakpointHi:
        return state.fifo.breakpoint >> 16;

    case CPRegister::FifoBreakpointLo:
        return state.fifo.breakpoint & 0xFFFF;

    default:
        Logger::log("CP", LogLevel::Warning,
                    "Unknown register read: " + utils::toHexString(offset));

        return 0;
    }
}

void CommandProcessor::write16(uint32_t offset, uint16_t value) {
    switch (static_cast<CPRegister>(offset)) {
    case CPRegister::Control: {
        bool wasFifoReadEnabled = state.fifoReadEnable;

        state.breakpointEnable = (value & (1 << 5)) != 0;

        state.fifoLinkEnable = (value & (1 << 4)) != 0;

        state.underflowInterruptEnable = (value & (1 << 3)) != 0;

        state.overflowInterruptEnable = (value & (1 << 2)) != 0;

        state.cpInterruptEnable = (value & (1 << 1)) != 0;

        state.fifoReadEnable = (value & (1 << 0)) != 0;

        if (!wasFifoReadEnabled && state.fifoReadEnable)
            Device::globalDevice->gx.initializeFifoReader();

        updateInterrupt();

        return;
    }

    case CPRegister::Clear:
        if (value & (1 << 1))
            state.underflow = false;

        if (value & (1 << 0))
            state.overflow = false;

        updateInterrupt();

        return;

    case CPRegister::FifoStartHi:
        writeHigh(state.fifo.base, value);
        return;

    case CPRegister::FifoStartLo:
        writeLow(state.fifo.base, value);
        return;

    case CPRegister::FifoEndHi:
        writeHigh(state.fifo.end, value);
        return;

    case CPRegister::FifoEndLo:
        writeLow(state.fifo.end, value);
        return;

    case CPRegister::FifoHighWatermarkHi:
        writeHigh(state.fifo.highWatermark, value);
        updateStatus();
        return;

    case CPRegister::FifoHighWatermarkLo:
        writeLow(state.fifo.highWatermark, value);
        updateStatus();
        return;

    case CPRegister::FifoLowWatermarkHi:
        writeHigh(state.fifo.lowWatermark, value);
        updateStatus();
        return;

    case CPRegister::FifoLowWatermarkLo:
        writeLow(state.fifo.lowWatermark, value);
        updateStatus();
        return;

    case CPRegister::FifoReadWriteDistanceHi:
        writeHigh(state.fifo.readWriteDistance, value);
        updateStatus();
        return;

    case CPRegister::FifoReadWriteDistanceLo:
        writeLow(state.fifo.readWriteDistance, value);
        updateStatus();
        return;

    case CPRegister::FifoWritePointerHi:
        writeHigh(state.fifo.writePointer, value);
        return;

    case CPRegister::FifoWritePointerLo:
        writeLow(state.fifo.writePointer, value);
        return;

    case CPRegister::FifoReadPointerHi:
        writeHigh(state.fifo.readPointer, value);
        return;

    case CPRegister::FifoReadPointerLo:
        writeLow(state.fifo.readPointer, value);
        return;

    case CPRegister::FifoBreakpointHi:
        writeHigh(state.fifo.breakpoint, value);
        return;

    case CPRegister::FifoBreakpointLo:
        writeLow(state.fifo.breakpoint, value);
        return;

    default:
        Logger::log("CP", LogLevel::Warning,
                    "Unknown register write: " + utils::toHexString(offset));
        return;
    }
}

uint32_t CommandProcessor::calculateFifoDistance() const {
    const auto &fifo = state.fifo;

    if (fifo.writePointer >= fifo.readPointer) {
        return fifo.writePointer - fifo.readPointer;
    }

    return (fifo.end - fifo.readPointer) + (fifo.writePointer - fifo.base) + 32;
}

void CommandProcessor::updateStatus() {
    state.overflow = state.fifo.readWriteDistance > state.fifo.highWatermark;
    state.underflow = state.fifo.readWriteDistance < state.fifo.lowWatermark;
    state.readIdle = state.fifo.readWriteDistance == 0;
    state.commandIdle =
        state.fifo.readWriteDistance == 0 || !state.fifoReadEnable;

    updateInterrupt();
}

void CommandProcessor::updateInterrupt() {
    bool breakpointInterrupt = state.bpInterrupt && state.cpInterruptEnable;

    bool overflowInterrupt = state.overflow && state.overflowInterruptEnable;

    bool underflowInterrupt = state.underflow && state.underflowInterruptEnable;

    bool interrupt =
        state.fifoReadEnable &&
        (breakpointInterrupt || overflowInterrupt || underflowInterrupt);

    if (interrupt) {
        Device::globalDevice->pi->raiseInterrupt(PIInterrupt::CP);
    } else {
        Device::globalDevice->pi->clearInterrupt(PIInterrupt::CP);
    }
}

void CommandProcessor::onGatherPipeBurst() {
    if (!state.fifoLinkEnable) {
        return;
    }

    if (state.fifo.writePointer == state.fifo.end) {
        state.fifo.writePointer = state.fifo.base;
    } else {
        state.fifo.writePointer += 32;
    }

    state.fifo.readWriteDistance += 32;

    if (state.fifoReadEnable)
        Device::globalDevice->gx.onFifoBytesAvailable(32);

    updateStatus();
}

void CommandProcessor::onFifoBlockConsumed() {
    if (state.fifo.readPointer == state.fifo.end)
        state.fifo.readPointer = state.fifo.base;
    else
        state.fifo.readPointer += 32;

    if (state.fifo.readWriteDistance >= 32)
        state.fifo.readWriteDistance -= 32;
    else
        state.fifo.readWriteDistance = 0;

    updateStatus();
}
