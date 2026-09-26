#include "core/system_hardware.h"
#include "device.h"
#include <algorithm>
#include <stdexcept>
#include <utility>

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
        return static_cast<uint16_t>(mailToDSP >> 16) |
               (mailToDSPReady ? 0x8000 : 0);
    case 0x02:
        return static_cast<uint16_t>(mailToDSP);
    case 0x04:
        return static_cast<uint16_t>(mailFromDSP >> 16) |
               (mailFromDSPReady ? 0x8000 : 0);
    case 0x06: {
        const uint16_t value = static_cast<uint16_t>(mailFromDSP);
        if (!pendingDSPMails.empty()) {
            mailFromDSP = pendingDSPMails.front();
            pendingDSPMails.pop_front();
            mailFromDSPReady = true;
        } else {
            mailFromDSPReady = false;
            mailFromDSP &= 0x7FFFFFFF;
        }
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
        mailToDSPReady = true;
        handleDSPMail(mailToDSP);
        mailToDSPReady = false;
        mailToDSP &= 0x7FFFFFFF;
        break;
    case 0x0A: {
        constexpr uint16_t pending = 0x00A8;
        constexpr uint16_t writable = 0x0D56;
        const bool wasHalted = (control & 4) != 0;
        control &= ~(value & pending);
        control = (control & ~writable) | (value & writable);
        control &= ~1u;
        if (value & 1u) {
            ++dspGeneration;
            dspHLEState = 0;
            dspBootCommand = 0;
            dspUploadWords = 0;
            asndVoiceAddress = 0;
            asndOutputAddress = 0;
            aesndParameterAddress = 0;
            aesndOutputAddress = 0;
            asndOutput.fill(0);
            aesndOutput.fill(0);
            pendingDSPMails.clear();
            mailFromDSPReady = false;
            queueDSPMail(0x8071FEED, false);
        }
        if (value & 0x0800)
            initCodeLoaded = true;
        if (wasHalted && !(control & 4) && initCodeLoaded) {
            ++dspGeneration;
            dspHLEState = 0;
            dspBootCommand = 0;
            pendingDSPMails.clear();
            mailFromDSPReady = false;
            queueDSPMail(0x80544348, false);
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

void DSPInterface::queueDSPMail(uint32_t mail, bool interrupt) {
    if (mailFromDSPReady)
        pendingDSPMails.push_back(mail);
    else {
        mailFromDSP = mail;
        mailFromDSPReady = true;
    }
    if (interrupt) {
        control |= 1u << 7;
        updateInterrupt();
    }
}

void DSPInterface::handleDSPMail(uint32_t mail) {
    if (dspHLEState == 0) {
        if (dspBootCommand == 0) {
            if ((mail & 0xFFFF0000u) == 0x80F30000u)
                dspBootCommand = mail;
            return;
        }
        const uint32_t command = dspBootCommand;
        dspBootCommand = 0;
        if (command == 0x80F3D001u) {
            dspHLEState = 1;
            queueDSPMail(0xDCD10000, true);
        }
        return;
    }

    if (dspHLEState == 3) {
        if (++dspUploadWords == 10) {
            dspUploadWords = 0;
            dspHLEState = 1;
            queueDSPMail(0xDCD10000, true);
        }
        return;
    }

    if (dspHLEState == 5) {
        asndVoiceAddress = mail;
        dspHLEState = 1;
        return;
    }

    if (dspHLEState == 6) {
        aesndParameterAddress = mail;
        dspHLEState = 1;
        return;
    }

    if (dspHLEState == 1) {
        if ((mail & 0xFFFF0000u) == 0xBABE0000u) {
            dspHLEState = 2;
            return;
        }
        if ((mail & 0xFFFFu) == 0x0123u) {
            dspHLEState = 5;
            return;
        }
        if ((mail & 0xFFFFu) == 0x0111u ||
            (mail & 0xFFFFu) == 0x0112u ||
            (mail & 0xFFFFu) == 0x0222u) {
            mixASND((mail & 0xFFFFu) == 0x0111u);
            queueDSPMail(0xDCD10004, true);
            return;
        }
        if ((mail & 0xFFFFu) == 0x0666u) {
            writeASNDOutput();
            queueDSPMail(0xDCD10004, true);
            return;
        }
        if ((mail & 0xFFFFu) == 0x0999u) {
            queueDSPMail(0xDCD10003, true);
            return;
        }
        if (mail == 0xFACE0080u) {
            dspHLEState = 6;
            return;
        }
        if (mail == 0xFACE0010u || mail == 0xFACE0020u) {
            mixAESND(mail == 0xFACE0010u);
            queueDSPMail(0xDCD10004, true);
            return;
        }
        if (mail == 0xFACE0100u) {
            writeAESNDOutput();
            queueDSPMail(0xDCD10004, true);
            return;
        }
        if (mail == 0xFACEDEADu) {
            queueDSPMail(0xDCD10003, true);
            return;
        }
        return;
    }

    if ((mail & 0xFFFF0000u) != 0xCDD10000u) {
        const uint32_t generation = dspGeneration;
        Device::globalDevice->scheduler.schedule(
            "DSPHLEWork",
            [this, generation]() {
                if (generation == dspGeneration)
                    queueDSPMail(0xDCD10002, true);
            },
            2500);
        dspHLEState = 4;
        return;
    }

    switch (mail) {
    case 0xCDD10000:
        dspHLEState = 1;
        queueDSPMail(0xDCD10001, true);
        break;
    case 0xCDD10001:
        dspHLEState = 3;
        dspUploadWords = 0;
        break;
    case 0xCDD10002:
        ++dspGeneration;
        dspHLEState = 0;
        dspBootCommand = 0;
        queueDSPMail(0x8071FEED, false);
        break;
    case 0xCDD10003:
        dspHLEState = 1;
        break;
    default:
        break;
    }
}

static std::pair<int16_t, int16_t> readHLEPCM(uint32_t address,
                                              uint32_t format,
                                              bool aram) {
    if (aram)
        address = 0x10000000u + (address & 0x03FFFFFFu);
    const auto byte = [&](uint32_t offset) {
        return static_cast<uint8_t>(Bus::readPhysical8(address + offset));
    };
    const auto be16 = [&](uint32_t offset) {
        return static_cast<int16_t>((uint16_t(byte(offset)) << 8) |
                                    byte(offset + 1));
    };
    const auto le16 = [&](uint32_t offset) {
        return static_cast<int16_t>(uint16_t(byte(offset)) |
                                    (uint16_t(byte(offset + 1)) << 8));
    };
    switch (format & 7u) {
    case 0: {
        const int16_t value = static_cast<int16_t>(
            int32_t(static_cast<int8_t>(byte(0))) * 256);
        return {value, value};
    }
    case 1: {
        const int16_t value = be16(0);
        return {value, value};
    }
    case 2:
        return {static_cast<int16_t>(
                    int32_t(static_cast<int8_t>(byte(0))) * 256),
                static_cast<int16_t>(
                    int32_t(static_cast<int8_t>(byte(1))) * 256)};
    case 3:
        return {be16(0), be16(2)};
    case 4: {
        const int16_t value = static_cast<int16_t>(
            (int32_t(byte(0)) - 128) * 256);
        return {value, value};
    }
    case 5: {
        const int16_t value = le16(0);
        return {value, value};
    }
    case 6:
        return {static_cast<int16_t>((int32_t(byte(0)) - 128) * 256),
                static_cast<int16_t>((int32_t(byte(1)) - 128) * 256)};
    case 7:
        return {le16(0), le16(2)};
    }
    return {};
}

static uint32_t hlePCMStride(uint32_t format) {
    static constexpr std::array<uint8_t, 8> strides{1, 2, 2, 4, 1, 2, 2, 4};
    return strides[format & 7u];
}

void DSPInterface::mixASND(bool clear) {
    if (clear)
        asndOutput.fill(0);
    if (asndVoiceAddress == 0)
        return;
    asndOutputAddress = Bus::readPhysical32(asndVoiceAddress);
    const uint32_t flags = Bus::readPhysical32(asndVoiceAddress + 8);
    uint32_t current = Bus::readPhysical32(asndVoiceAddress + 12);
    const uint32_t end = Bus::readPhysical32(asndVoiceAddress + 16);
    const uint32_t frequency = Bus::readPhysical32(asndVoiceAddress + 20);
    uint32_t counter = Bus::readPhysical32(asndVoiceAddress + 28);
    const uint32_t volumeLeft = Bus::readPhysical16(asndVoiceAddress + 32);
    const uint32_t volumeRight = Bus::readPhysical16(asndVoiceAddress + 34);
    const uint32_t loopStart = Bus::readPhysical32(asndVoiceAddress + 48);
    const uint32_t format = flags & 7u;
    uint32_t stride = flags >> 16;
    if (stride == 0)
        stride = hlePCMStride(format);
    const bool newFlags = format >= 4 || (flags & 0x300u) != 0;
    const bool paused = (flags & (newFlags ? 1u << 9 : 1u << 5)) != 0;
    const bool looping = (flags & (newFlags ? 1u << 8 : 1u << 2)) != 0;
    int16_t lastLeft = static_cast<int16_t>(
        Bus::readPhysical16(asndVoiceAddress + 24));
    int16_t lastRight = static_cast<int16_t>(
        Bus::readPhysical16(asndVoiceAddress + 26));
    if (!paused && current != 0 && end > current) {
        for (uint32_t frame = 0; frame < 1024; ++frame) {
            while (counter >= 48000 && current != 0) {
                counter -= 48000;
                current += stride;
                if (current >= end)
                    current = looping ? loopStart : 0;
            }
            if (current == 0)
                break;
            const auto [left, right] = readHLEPCM(current, format, false);
            lastLeft = static_cast<int16_t>(
                (int32_t(left) * int32_t(volumeLeft)) / 256);
            lastRight = static_cast<int16_t>(
                (int32_t(right) * int32_t(volumeRight)) / 256);
            asndOutput[frame * 2] = static_cast<int16_t>(std::clamp(
                int32_t(asndOutput[frame * 2]) + lastRight, -32768, 32767));
            asndOutput[frame * 2 + 1] = static_cast<int16_t>(std::clamp(
                int32_t(asndOutput[frame * 2 + 1]) + lastLeft, -32768, 32767));
            counter += frequency;
        }
    }
    Bus::writePhysical32(asndVoiceAddress + 12, current);
    Bus::writePhysical16(asndVoiceAddress + 24,
                         static_cast<uint16_t>(lastLeft));
    Bus::writePhysical16(asndVoiceAddress + 26,
                         static_cast<uint16_t>(lastRight));
    Bus::writePhysical32(asndVoiceAddress + 28, counter);
}

void DSPInterface::writeASNDOutput() {
    if (asndOutputAddress == 0)
        return;
    for (size_t i = 0; i < asndOutput.size(); ++i)
        Bus::writePhysical16(asndOutputAddress + i * 2,
                             static_cast<uint16_t>(asndOutput[i]));
}

void DSPInterface::mixAESND(bool clear) {
    if (clear)
        aesndOutput.fill(0);
    if (aesndParameterAddress == 0)
        return;
    const uint32_t base = aesndParameterAddress;
    const uint32_t output = Bus::readPhysical32(base);
    const uint32_t start = Bus::readPhysical32(base + 4);
    const uint32_t end = Bus::readPhysical32(base + 8);
    uint32_t current = Bus::readPhysical32(base + 12);
    const uint32_t frequency =
        (uint32_t(Bus::readPhysical16(base + 22)) << 16) |
        Bus::readPhysical16(base + 24);
    uint32_t counter = Bus::readPhysical16(base + 26);
    const uint32_t volumeLeft = Bus::readPhysical16(base + 32);
    const uint32_t volumeRight = Bus::readPhysical16(base + 34);
    uint32_t flags = Bus::readPhysical32(base + 40);
    const uint32_t format = flags & 7u;
    static constexpr std::array<uint8_t, 8> aesndFormats{0, 2, 1, 3,
                                                         4, 6, 1, 3};
    const uint32_t pcmFormat = aesndFormats[format];
    const uint32_t stride = hlePCMStride(pcmFormat);
    const bool newFlags = format >= 4 || (flags & 0x20u) != 0;
    const bool paused = (flags & (newFlags ? 8u : 4u)) != 0;
    const bool looping = (flags & (newFlags ? 0x10u : 8u)) != 0;
    const bool running = (flags & 0x40000000u) != 0;
    if (!paused && running && start != 0 && current < end) {
        for (uint32_t frame = 0; frame < 96; ++frame) {
            const auto [left, right] = readHLEPCM(current, pcmFormat, true);
            const int32_t mixedLeft =
                (int32_t(left) * int32_t(volumeLeft)) / 256;
            const int32_t mixedRight =
                (int32_t(right) * int32_t(volumeRight)) / 256;
            aesndOutput[frame * 2] = static_cast<int16_t>(std::clamp(
                int32_t(aesndOutput[frame * 2]) + mixedRight, -32768, 32767));
            aesndOutput[frame * 2 + 1] = static_cast<int16_t>(std::clamp(
                int32_t(aesndOutput[frame * 2 + 1]) + mixedLeft, -32768, 32767));
            counter += frequency;
            while (counter >= 0x10000u) {
                counter -= 0x10000u;
                current += stride;
                if (current >= end) {
                    if (looping) {
                        current = start;
                    } else {
                        flags &= ~0x40000000u;
                        flags |= 0x00100000u;
                        current = end;
                    }
                    break;
                }
            }
            if (!(flags & 0x40000000u))
                break;
        }
    }
    aesndOutputAddress = output;
    Bus::writePhysical32(base + 12, current);
    Bus::writePhysical16(base + 26, static_cast<uint16_t>(counter));
    Bus::writePhysical32(base + 40, flags);
}

void DSPInterface::writeAESNDOutput() {
    if (aesndOutputAddress == 0)
        return;
    for (size_t i = 0; i < aesndOutput.size(); ++i)
        Bus::writePhysical16(aesndOutputAddress + i * 2,
                             static_cast<uint16_t>(aesndOutput[i]));
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
