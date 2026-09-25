#include "core/system_hardware.h"
#include "device.h"
#include <stdexcept>

uint32_t DiscInterface::read(uint32_t offset, AccessSize size) {
    constexpr uint32_t config = 1;
    if (offset < 0x24 || offset >= 0x28)
        return 0;
    if (size == AccessSize::U32 && offset == 0x24)
        return config;
    if (size == AccessSize::U16 && (offset == 0x24 || offset == 0x26))
        return offset == 0x24 ? config >> 16 : config & 0xFFFF;
    if (size == AccessSize::U8)
        return (config >> ((3 - (offset & 3)) * 8)) & 0xFF;
    return 0;
}

void DiscInterface::write(uint32_t, uint32_t, AccessSize) {}

uint16_t DSPInterface::read16(uint32_t offset) {
    switch (offset) {
    case 0x00:
        return static_cast<uint16_t>(mailToDSP >> 16);
    case 0x02:
        return static_cast<uint16_t>(mailToDSP);
    case 0x04:
        return static_cast<uint16_t>(mailFromDSP >> 16) |
               (mailFromDSPReady ? 0x8000 : 0);
    case 0x06: {
        const uint16_t value = static_cast<uint16_t>(mailFromDSP);
        mailFromDSPReady = false;
        return value;
    }
    case 0x0A:
        return control;
    case 0x10:
        return interruptControl;
    case 0x12:
        return arInfo;
    case 0x16:
        return 1;
    case 0x1A:
        return arRefresh;
    case 0x20:
        return static_cast<uint16_t>(arMainAddress >> 16);
    case 0x22:
        return static_cast<uint16_t>(arMainAddress);
    case 0x24:
        return static_cast<uint16_t>(arAddress >> 16);
    case 0x26:
        return static_cast<uint16_t>(arAddress);
    case 0x28:
        return static_cast<uint16_t>(arCount >> 16);
    case 0x2A:
        return static_cast<uint16_t>(arCount);
    case 0x30:
        return static_cast<uint16_t>(audioAddress >> 16);
    case 0x32:
        return static_cast<uint16_t>(audioAddress);
    case 0x36:
        return audioControl;
    case 0x3A:
        return audioBlocksLeft > 0 ? audioBlocksLeft - 1 : 0;
    default:
        return 0;
    }
}

uint32_t DSPInterface::read(uint32_t offset, AccessSize size) {
    if (size == AccessSize::U16)
        return read16(offset);
    if (size == AccessSize::U32)
        return (static_cast<uint32_t>(read16(offset)) << 16) |
               read16(offset + 2);
    if (size == AccessSize::U8) {
        const uint16_t value = read16(offset & ~1u);
        return (offset & 1) ? value & 0xFF : value >> 8;
    }
    throw std::runtime_error("Unsupported DSP interface access size");
}

void DSPInterface::write16(uint32_t offset, uint16_t value) {
    switch (offset) {
    case 0x00:
        mailToDSP = (mailToDSP & 0xFFFF) | (static_cast<uint32_t>(value) << 16);
        break;
    case 0x02:
        mailToDSP = (mailToDSP & 0xFFFF0000) | value;
        break;
    case 0x0A: {
        constexpr uint16_t pending = 0x00A8;
        constexpr uint16_t writable = 0x0D56;
        const bool wasHalted = (control & 4) != 0;
        control &= ~(value & pending);
        control = (control & ~writable) | (value & writable);
        control &= ~1u;
        if (value & 0x0800)
            initCodeLoaded = true;
        if (wasHalted && !(control & 4) && initCodeLoaded) {
            mailFromDSP = 0x00544348;
            mailFromDSPReady = true;
            initCodeLoaded = false;
        }
        updateInterrupt();
        break;
    }
    case 0x10:
        interruptControl = value;
        break;
    case 0x12:
        arInfo = value & 0x7F;
        break;
    case 0x1A:
        arRefresh = value & 0x7FF;
        break;
    case 0x20:
        arMainAddress = (arMainAddress & 0xFFFF) |
                        (static_cast<uint32_t>(value & 0x03FF) << 16);
        break;
    case 0x22:
        arMainAddress = (arMainAddress & 0xFFFF0000) | (value & 0xFFE0);
        break;
    case 0x24:
        arAddress = (arAddress & 0xFFFF) |
                    (static_cast<uint32_t>(value & 0x03FF) << 16);
        break;
    case 0x26:
        arAddress = (arAddress & 0xFFFF0000) | (value & 0xFFE0);
        break;
    case 0x28:
        arCount =
            (arCount & 0xFFFF) | (static_cast<uint32_t>(value & 0x83FF) << 16);
        break;
    case 0x2A:
        arCount = (arCount & 0xFFFF0000) | (value & 0xFFE0);
        completeARAMTransfer();
        break;
    case 0x30:
        audioAddress =
            (audioAddress & 0xFFFF) | (uint32_t(value & 0x1FFF) << 16);
        break;
    case 0x32:
        audioAddress = (audioAddress & 0xFFFF0000) | (value & 0xFFE0);
        break;
    case 0x36: {
        const bool starting = !(audioControl & 0x8000) && (value & 0x8000);
        audioControl = value;
        if (starting) {
            audioCursor = audioAddress;
            audioBlocksLeft = value & 0x7FFF;
            audioAccumulator = 0;
            Device::globalDevice->scheduler.schedule(
                "AIDMAStart",
                [this]() {
                    if (audioControl & 0x8000) {
                        control |= 1u << 3;
                        updateInterrupt();
                    }
                },
                200);
        }
        if (!(value & 0x8000)) {
            audioBlocksLeft = 0;
            audioAccumulator = 0;
        }
        break;
    }
    default:
        break;
    }
}

void DSPInterface::completeARAMTransfer() {
    const bool fromARAM = (arCount & 0x80000000u) != 0;
    const uint32_t size = arCount & 0x03FFFFE0u;
    control |= 1u << 9;
    for (uint32_t offset = 0; offset < size; ++offset) {
        const uint32_t main = (arMainAddress + offset) & 0x03FFFFFFu;
        const uint32_t aram =
            0x10000000u + ((arAddress + offset) & 0x03FFFFFFu);
        if (fromARAM)
            Bus::writePhysical8(main, Bus::readPhysical8(aram));
        else
            Bus::writePhysical8(aram, Bus::readPhysical8(main));
    }
    arMainAddress += size;
    arAddress += size;
    arCount &= 0x80000000u;
    control &= ~(1u << 9);
    control |= 1u << 5;
    updateInterrupt();
}

void DSPInterface::updateInterrupt() {
    const bool pending = ((control & (1u << 3)) && (control & (1u << 4))) ||
                         ((control & (1u << 5)) && (control & (1u << 6))) ||
                         ((control & (1u << 7)) && (control & (1u << 8)));
    if (pending)
        Device::globalDevice->pi->raiseInterrupt(PIInterrupt::DSP);
    else
        Device::globalDevice->pi->clearInterrupt(PIInterrupt::DSP);
}

void DSPInterface::write(uint32_t offset, uint32_t value, AccessSize size) {
    if (size == AccessSize::U16) {
        write16(offset, static_cast<uint16_t>(value));
        return;
    }
    if (size == AccessSize::U32) {
        write16(offset, static_cast<uint16_t>(value >> 16));
        write16(offset + 2, static_cast<uint16_t>(value));
        return;
    }
    if (size == AccessSize::U8) {
        const uint32_t aligned = offset & ~1u;
        uint16_t merged = read16(aligned);
        if (offset & 1)
            merged = (merged & 0xFF00) | (value & 0xFF);
        else
            merged = (merged & 0x00FF) | ((value & 0xFF) << 8);
        write16(aligned, merged);
        return;
    }
    throw std::runtime_error("Unsupported DSP interface access size");
}

ExpansionInterface::ExpansionInterface() {
    status[0] = 1u << 11;
    status[1] = (1u << 11) | (1u << 7);
}

uint32_t ExpansionInterface::read(uint32_t offset, AccessSize size) {
    if (size != AccessSize::U32 || offset >= EXI_SIZE)
        return 0;
    const uint32_t channel = offset / 0x14;
    const uint32_t reg = offset % 0x14;
    if (channel >= 3)
        return 0;
    switch (reg) {
    case 0x00:
        return status[channel];
    case 0x04:
        return dmaAddress[channel];
    case 0x08:
        return dmaLength[channel];
    case 0x0C:
        return control[channel];
    case 0x10:
        return immediateData[channel];
    default:
        return 0;
    }
}

void ExpansionInterface::write(uint32_t offset, uint32_t value,
                               AccessSize size) {
    if (size != AccessSize::U32 || offset >= EXI_SIZE)
        return;
    const uint32_t channel = offset / 0x14;
    const uint32_t reg = offset % 0x14;
    if (channel >= 3)
        return;
    switch (reg) {
    case 0x00: {
        constexpr uint32_t flags = (1u << 1) | (1u << 3) | (1u << 11);
        constexpr uint32_t writable =
            1u | (1u << 2) | 0x70u | 0x380u | (1u << 10) | (1u << 13);
        status[channel] &= ~(value & flags);
        status[channel] = (status[channel] & ~writable) | (value & writable);
        break;
    }
    case 0x04:
        dmaAddress[channel] = value;
        break;
    case 0x08:
        dmaLength[channel] = value;
        break;
    case 0x0C:
        control[channel] = value;
        if (value & 1) {
            control[channel] &= ~1u;
            status[channel] |= 1u << 3;
            if ((value & 2) == 0 && (value & 0x0C) == 0)
                immediateData[channel] = 0xFFFFFFFF;
        }
        break;
    case 0x10:
        immediateData[channel] = value;
        break;
    default:
        break;
    }
    updateInterrupt();
}

void ExpansionInterface::updateInterrupt() {
    bool pending = false;
    for (uint32_t value : status) {
        pending |= ((value & 1) && (value & 2)) ||
                   ((value & 4) && (value & 8)) ||
                   ((value & (1u << 10)) && (value & (1u << 11)));
    }
    if (pending)
        Device::globalDevice->pi->raiseInterrupt(PIInterrupt::EXI);
    else
        Device::globalDevice->pi->clearInterrupt(PIInterrupt::EXI);
}

void DSPInterface::step(uint32_t cycles) {
    if (!(audioControl & 0x8000))
        return;
    auto &device = *Device::globalDevice;
    const uint32_t rate = device.ai->dmaSampleRate();
    audioAccumulator += uint64_t(cycles) * rate;
    constexpr uint64_t blockPeriod = BROADWAY_CLOCK * 8;
    while (audioAccumulator >= blockPeriod) {
        audioAccumulator -= blockPeriod;
        std::array<int16_t, 16> samples{};
        for (uint32_t frame = 0; frame < 8; ++frame) {
            samples[frame * 2] = static_cast<int16_t>(
                Bus::readPhysical16(audioCursor + frame * 4 + 2));
            samples[frame * 2 + 1] = static_cast<int16_t>(
                Bus::readPhysical16(audioCursor + frame * 4));
        }
        device.audio.submitSamples(samples, rate);
        if (audioBlocksLeft != 0) {
            --audioBlocksLeft;
            audioCursor += 32;
        }
        if (audioBlocksLeft == 0) {
            audioCursor = audioAddress;
            audioBlocksLeft = audioControl & 0x7FFF;
            control |= 1u << 3;
            updateInterrupt();
        }
    }
}
