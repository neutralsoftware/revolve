#include "input/wiimote.h"

std::array<uint8_t, 2> WiiRemoteDevice::makeButtons() const {
    uint16_t buttons = 0;

    if (state->dpadLeft)
        buttons |= 0x0001;
    if (state->dpadRight)
        buttons |= 0x0002;
    if (state->dpadDown)
        buttons |= 0x0004;
    if (state->dpadUp)
        buttons |= 0x0008;

    if (state->plus)
        buttons |= 0x0010;

    if (state->two)
        buttons |= 0x0100;
    if (state->one)
        buttons |= 0x0200;
    if (state->b)
        buttons |= 0x0400;
    if (state->a)
        buttons |= 0x0800;
    if (state->minus)
        buttons |= 0x1000;
    if (state->home)
        buttons |= 0x8000;

    return {static_cast<uint8_t>(buttons >> 8),
            static_cast<uint8_t>(buttons & 0xFF)};
}

static uint16_t accelTo10Bit(float g) {
    constexpr float zero = 512.0f;
    constexpr float oneG = 128.0f;

    int value = static_cast<int>(zero + g * oneG);

    return static_cast<uint16_t>(std::clamp(value, 0, 1023));
}

std::array<uint8_t, 3> WiiRemoteDevice::makeAccel() const {
    uint16_t x = accelTo10Bit(state->accelX);
    uint16_t y = accelTo10Bit(state->accelY);
    uint16_t z = accelTo10Bit(state->accelZ);

    return {static_cast<uint8_t>(x >> 2), static_cast<uint8_t>(y >> 2),
            static_cast<uint8_t>(z >> 2)};
}

std::array<uint8_t, 6> WiiRemoteDevice::makeNunchuk() const {
    const auto &n = state->nunchuk;

    uint16_t ax = accelTo10Bit(n.accelX);
    uint16_t ay = accelTo10Bit(n.accelY);
    uint16_t az = accelTo10Bit(n.accelZ);

    std::array<uint8_t, 6> out{};

    out[0] = n.stickX;
    out[1] = n.stickY;

    out[2] = static_cast<uint8_t>(ax >> 2);
    out[3] = static_cast<uint8_t>(ay >> 2);
    out[4] = static_cast<uint8_t>(az >> 2);

    out[5] = ((az & 0x3) << 6) | ((ay & 0x3) << 4) | ((ax & 0x3) << 2);

    if (!n.c)
        out[5] |= 0x02;

    if (!n.z)
        out[5] |= 0x01;

    return out;
}

void initializeNunchukRegisters(std::array<uint8_t, 256> &regs) {
    regs.fill(0);

    regs[0xFA] = 0x00;
    regs[0xFB] = 0x00;
    regs[0xFC] = 0xA4;
    regs[0xFD] = 0x20;
    regs[0xFE] = 0x00;
    regs[0xFF] = 0x00;
}

uint8_t WiiRemoteDevice::readRegister(uint32_t address) {
    if ((address & 0xFF0000) == 0xA40000) {
        uint8_t offset = address & 0xFF;

        if (offset >= 0x08 && offset < 0x0E) {
            if (!state->nunchukConnected)
                return 0xFF;

            auto data = makeNunchuk();
            return data[offset - 0x08];
        }

        return extensionRegisters[offset];
    }

    if ((address & 0xFF0000) == 0xB00000) {
        uint8_t offset = address & 0xFF;

        if (offset < irRegisters.size())
            return irRegisters[offset];

        return 0;
    }

    return 0;
}

void WiiRemoteDevice::writeRegister(uint32_t address, uint8_t value) {
    if ((address & 0xFF0000) == 0xA40000) {
        uint8_t offset = address & 0xFF;

        extensionRegisters[offset] = value;

        if (offset == 0xF0 && value == 0x55) {
        }

        if (offset == 0xFB && value == 0x00) {
        }

        return;
    }

    if ((address & 0xFF0000) == 0xA60000) {
        // MotionPlus later
    }

    if ((address & 0xFF0000) == 0xB00000) {
        uint8_t offset = address & 0xFF;

        if (offset < irRegisters.size())
            irRegisters[offset] = value;

        return;
    }
}

std::array<uint8_t, 12> WiiRemoteDevice::makeIRExtended() const {
    std::array<uint8_t, 12> out{};
    out.fill(0xFF);

    for (size_t i = 0; i < 4; ++i) {
        const auto &p = state->ir[i];

        if (!p.visible)
            continue;

        size_t base = i * 3;

        out[base + 0] = static_cast<uint8_t>(p.x & 0xFF);
        out[base + 1] = static_cast<uint8_t>(p.y & 0xFF);
        out[base + 2] =
            static_cast<uint8_t>(((p.y >> 8) & 0x3) << 6 |
                                 ((p.x >> 8) & 0x3) << 4 | (p.size & 0x0F));
    }

    return out;
}

std::array<uint8_t, 10> WiiRemoteDevice::makeIRBasic() const {
    std::array<uint8_t, 10> out{};
    out.fill(0xFF);

    for (int pair = 0; pair < 2; ++pair) {
        const auto &a = state->ir[pair * 2 + 0];
        const auto &b = state->ir[pair * 2 + 1];

        size_t base = pair * 5;

        if (!a.visible && !b.visible)
            continue;

        uint16_t ax = a.visible ? a.x : 0x3FF;
        uint16_t ay = a.visible ? a.y : 0x3FF;

        uint16_t bx = b.visible ? b.x : 0x3FF;
        uint16_t by = b.visible ? b.y : 0x3FF;

        out[base + 0] = ax & 0xFF;
        out[base + 1] = ay & 0xFF;

        out[base + 2] = ((ay >> 8) & 0x3) << 6 | ((ax >> 8) & 0x3) << 4 |
                        ((by >> 8) & 0x3) << 2 | ((bx >> 8) & 0x3);

        out[base + 3] = bx & 0xFF;
        out[base + 4] = by & 0xFF;
    }

    return out;
}

void WiiRemoteDevice::queueDataReport() {
    if (!state || !state->connected)
        return;

    const auto buttons = makeButtons();
    const auto accel = makeAccel();

    std::vector<uint8_t> report;

    report.push_back(reportMode);

    switch (reportMode) {
    case 0x30:
        report.insert(report.end(), buttons.begin(), buttons.end());
        break;

    case 0x31:
        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        break;

    case 0x32: {
        auto ext = makeNunchuk();

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), ext.begin(), ext.end());

        while (report.size() < 1 + 2 + 8)
            report.push_back(0);
        break;
    }

    case 0x33: {
        auto ir = makeIRExtended();

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        report.insert(report.end(), ir.begin(), ir.end());
        break;
    }

    case 0x35: {
        auto ext = makeNunchuk();

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        report.insert(report.end(), ext.begin(), ext.end());

        while (report.size() < 1 + 2 + 3 + 16)
            report.push_back(0);
        break;
    }

    case 0x37: {
        auto ir = makeIRBasic();
        auto ext = makeNunchuk();

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        report.insert(report.end(), ir.begin(), ir.end());
        report.insert(report.end(), ext.begin(), ext.end());
        break;
    }

    default:
        return;
    }

    inputQueue.push_back(std::move(report));
}

void WiiRemoteDevice::handleOutputReport(uint8_t reportId,
                                         std::span<const uint8_t> payload) {
    if (payload.empty() && reportId != 0x15)
        return;

    switch (reportId) {
    case 0x10:
        if (!payload.empty())
            rumble = payload[0] & 0x01;
        break;

    case 0x11:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            leds = payload[0] & 0xF0;
        }
        break;

    case 0x12:
        if (payload.size() >= 2) {
            rumble = payload[0] & 0x01;
            continuousReporting = (payload[0] & 0x04) != 0;

            reportMode = payload[1];
        }
        break;

    case 0x13:
    case 0x1A:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            irEnabled = (payload[0] & 0x04) != 0;
        }
        break;

    case 0x14:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            speakerEnabled = (payload[0] & 0x04) != 0;
        }
        break;

    case 0x15:
        queueStatus();
        break;

    case 0x16:
        handleWriteMemory(payload);
        break;

    case 0x17:
        handleReadMemory(payload);
        break;

    default:
        break;
    }

    if (!payload.empty() && (payload[0] & 0x02))
        queueAck(reportId);
}

void WiiRemoteDevice::queueStatus() {
    auto buttons = makeButtons();

    uint8_t flags = leds;

    if (state->nunchukConnected)
        flags |= 0x02;

    if (speakerEnabled)
        flags |= 0x04;

    if (irEnabled)
        flags |= 0x08;

    inputQueue.push_back(
        {0x20, buttons[0], buttons[1], flags, 0x00, 0x00, battery});
}

void WiiRemoteDevice::queueAck(uint8_t report, uint8_t error) {
    auto buttons = makeButtons();

    inputQueue.push_back({0x22, buttons[0], buttons[1], report, error});
}

void WiiRemoteDevice::handleWriteMemory(std::span<const uint8_t> payload) {
    if (payload.size() < 5)
        return;

    const bool registers = (payload[0] & 0x04) != 0;

    uint32_t address = (uint32_t(payload[1]) << 16) |
                       (uint32_t(payload[2]) << 8) | uint32_t(payload[3]);

    uint8_t size = payload[4];

    size = std::min<uint8_t>(size, static_cast<uint8_t>(payload.size() - 5));

    for (uint8_t i = 0; i < size; ++i) {
        if (registers) {
            writeRegister(address + i, payload[5 + i]);
        } else {
            if (address + i < eeprom.size())
                eeprom[address + i] = payload[5 + i];
        }
    }
}

void WiiRemoteDevice::handleReadMemory(std::span<const uint8_t> payload) {
    if (payload.size() < 6)
        return;

    const bool registers = (payload[0] & 0x04) != 0;
    uint32_t address = (uint32_t(payload[1]) << 16) |
                       (uint32_t(payload[2]) << 8) | uint32_t(payload[3]);

    uint16_t size = (uint16_t(payload[4]) << 8) | uint16_t(payload[5]);

    while (size > 0) {
        uint8_t chunk = std::min<uint16_t>(size, 16);

        auto buttons = makeButtons();

        std::vector<uint8_t> response(22, 0);

        response[0] = 0x21;
        response[1] = buttons[0];
        response[2] = buttons[1];

        response[3] = static_cast<uint8_t>((chunk - 1) << 4);

        response[4] = static_cast<uint8_t>(address >> 8);

        response[5] = static_cast<uint8_t>(address);

        for (uint8_t i = 0; i < chunk; ++i) {
            uint8_t value = 0;

            if (registers) {
                value = readRegister(address + i);
            } else if (address + i < eeprom.size()) {
                value = eeprom[address + i];
            }

            response[6 + i] = value;
        }

        inputQueue.push_back(std::move(response));

        address += chunk;
        size -= chunk;
    }
}

void WiiRemoteDevice::update() {
    if (!state || !state->connected)
        return;

    if (continuousReporting)
        queueDataReport();
}

std::vector<uint8_t> WiiRemoteDevice::popInputReport() {
    if (inputQueue.empty())
        return {};

    auto result = std::move(inputQueue.front());

    inputQueue.pop_front();

    return result;
}