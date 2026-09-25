#include "input/wiimote.h"
#include "input/gamecube.h"
#include <algorithm>
#include <vector>

WiiRemoteDevice::WiiRemoteDevice(WiiRemoteState *state) : state(state) {
    initializeEEPROM();
    initializeNunchukRegisters();
    initializeMotionPlusRegisters();
}

static uint16_t accelTo10Bit(float g) {
    constexpr float zero = 512.0f;
    constexpr float oneG = 128.0f;

    int value = static_cast<int>(zero + g * oneG);

    return static_cast<uint16_t>(std::clamp(value, 0, 1023));
}

void WiiRemoteDevice::initializeEEPROM() {
    eeprom.fill(0);

    constexpr uint8_t irCal[] = {0xA1, 0xAA, 0x8B, 0x99, 0xAE, 0x9E,
                                 0x78, 0x30, 0xA7, 0x74, 0xD3};

    std::copy(std::begin(irCal), std::end(irCal), eeprom.begin() + 0x0000);
    std::copy(std::begin(irCal), std::end(irCal), eeprom.begin() + 0x000B);

    constexpr uint8_t accelCal[] = {0x80, 0x80, 0x80, 0x00, 0xA0,
                                    0xA0, 0xA0, 0x00, 0x40, 0xF5};

    std::copy(std::begin(accelCal), std::end(accelCal),
              eeprom.begin() + 0x0016);
    std::copy(std::begin(accelCal), std::end(accelCal),
              eeprom.begin() + 0x0020);
}

std::array<uint8_t, 2> WiiRemoteDevice::makeButtons(bool includeAccel) const {
    uint8_t b0 = 0;
    uint8_t b1 = 0;

    if (state->dpadLeft)
        b0 |= 0x01;
    if (state->dpadRight)
        b0 |= 0x02;
    if (state->dpadDown)
        b0 |= 0x04;
    if (state->dpadUp)
        b0 |= 0x08;
    if (state->plus)
        b0 |= 0x10;

    if (state->two)
        b1 |= 0x01;
    if (state->one)
        b1 |= 0x02;
    if (state->b)
        b1 |= 0x04;
    if (state->a)
        b1 |= 0x08;
    if (state->minus)
        b1 |= 0x10;
    if (state->home)
        b1 |= 0x80;

    if (includeAccel) {
        const uint16_t ax = accelTo10Bit(state->accelX);
        const uint16_t ay = accelTo10Bit(state->accelY);
        const uint16_t az = accelTo10Bit(state->accelZ);

        b0 |= static_cast<uint8_t>((ax & 0x3) << 5);

        b1 |= static_cast<uint8_t>(((ay >> 1) & 1) << 5);
        b1 |= static_cast<uint8_t>(((az >> 1) & 1) << 6);
    }

    return {b0, b1};
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

void WiiRemoteDevice::initializeNunchukRegisters() {
    auto &regs = extensionRegisters;
    regs.fill(0);

    regs[0xFA] = 0x00;
    regs[0xFB] = 0x00;
    regs[0xFC] = 0xA4;
    regs[0xFD] = 0x20;
    regs[0xFE] = 0x00;
    regs[0xFF] = 0x00;
}

void WiiRemoteDevice::initializeMotionPlusRegisters() {
    motionPlusRegisters.fill(0);
    motionPlusRegisters[0xFC] = 0xA6;
    motionPlusRegisters[0xFD] = 0x20;
    motionPlusRegisters[0xFF] = 0x05;
}

std::array<uint8_t, 3> WiiRemoteDevice::makeAccel() const {
    return {
        static_cast<uint8_t>(accelTo10Bit(state->accelX) >> 2),
        static_cast<uint8_t>(accelTo10Bit(state->accelY) >> 2),
        static_cast<uint8_t>(accelTo10Bit(state->accelZ) >> 2),
    };
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

    if ((address & 0xFF0000) == 0xA60000)
        return motionPlusRegisters[address & 0xFF];

    return 0;
}

void WiiRemoteDevice::writeRegister(uint32_t address, uint8_t value) {
    if ((address & 0xFF0000) == 0xA40000) {
        uint8_t offset = address & 0xFF;

        extensionRegisters[offset] = value;

        return;
    }

    if ((address & 0xFF0000) == 0xA60000) {
        motionPlusRegisters[address & 0xFF] = value;
        return;
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

std::vector<uint8_t> WiiRemoteDevice::buildDataReport() {
    if (!state || !state->connected)
        return {};

    std::vector<uint8_t> report;

    report.push_back(reportMode);

    switch (reportMode) {
    case 0x30: {
        auto buttons = makeButtons(false);
        report.insert(report.end(), buttons.begin(), buttons.end());
        break;
    }

    case 0x31: {
        auto buttons = makeButtons(true);
        auto accel = makeAccel();
        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        break;
    }

    case 0x34: {
        auto buttons = makeButtons(false);
        auto ext = makeNunchuk();
        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), ext.begin(), ext.end());
        while (report.size() < 1 + 2 + 19)
            report.push_back(0);
        break;
    }

    case 0x32: {
        auto ext = makeNunchuk();

        auto buttons = makeButtons(false);

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), ext.begin(), ext.end());

        while (report.size() < 1 + 2 + 8)
            report.push_back(0);
        break;
    }

    case 0x36: {
        auto buttons = makeButtons(false);
        auto ir = makeIRBasic();
        auto ext = makeNunchuk();
        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), ir.begin(), ir.end());
        report.insert(report.end(), ext.begin(), ext.end());
        while (report.size() < 1 + 2 + 10 + 9)
            report.push_back(0);
        break;
    }

    case 0x33: {
        auto ir = makeIRExtended();
        auto buttons = makeButtons(true);
        auto accel = makeAccel();

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        report.insert(report.end(), ir.begin(), ir.end());
        break;
    }

    case 0x35: {
        auto ext = makeNunchuk();
        auto buttons = makeButtons(true);
        auto accel = makeAccel();

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
        auto buttons = makeButtons(true);
        auto accel = makeAccel();

        report.insert(report.end(), buttons.begin(), buttons.end());
        report.insert(report.end(), accel.begin(), accel.end());
        report.insert(report.end(), ir.begin(), ir.end());
        report.insert(report.end(), ext.begin(), ext.end());
        break;
    }

    case 0x3D: {
        auto ext = makeNunchuk();
        report.insert(report.end(), ext.begin(), ext.end());
        while (report.size() < 1 + 21)
            report.push_back(0);
        break;
    }

    default:
        return {};
    }

    return report;
}

void WiiRemoteDevice::handleOutputReport(uint8_t reportId,
                                         std::span<const uint8_t> payload) {
    if (payload.empty() && reportId != 0x15)
        return;

    switch (reportId) {
    case 0x10:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            state->rumble = rumble;
        }
        break;

    case 0x11:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            leds = payload[0] & 0xF0;
            state->rumble = rumble;
            state->leds = leds;
        }
        break;

    case 0x12:
        if (payload.size() >= 2) {
            rumble = payload[0] & 0x01;
            state->rumble = rumble;
            continuousReporting = payload[0] & 0x04;
            reportMode = payload[1];

            dataReportingEnabled = true;
            lastDataReport.clear();
        }
        break;

    case 0x13:
    case 0x1A:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            state->rumble = rumble;
            irEnabled = (payload[0] & 0x04) != 0;
        }
        break;

    case 0x14:
        if (!payload.empty()) {
            rumble = payload[0] & 0x01;
            state->rumble = rumble;
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

    if (++updateCounter < 4096)
        return;
    updateCounter = 0;

    if (state->nunchukConnected != previousNunchukConnected) {
        previousNunchukConnected = state->nunchukConnected;

        queueStatus();

        dataReportingEnabled = false;
        return;
    }

    if (!dataReportingEnabled)
        return;

    auto report = buildDataReport();

    if (report.empty())
        return;

    if (continuousReporting || report != lastDataReport) {
        inputQueue.push_back(report);
        lastDataReport = std::move(report);
    }
}

std::vector<uint8_t> WiiRemoteDevice::popInputReport() {
    if (inputQueue.empty())
        return {};

    auto result = std::move(inputQueue.front());

    inputQueue.pop_front();

    return result;
}

SDLWiiRemoteInput::~SDLWiiRemoteInput() { shutdown(); }

bool SDLWiiRemoteInput::initialize(std::size_t index) {
    shutdown();
    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);
    if (!ids || index >= static_cast<std::size_t>(count)) {
        SDL_free(ids);
        return false;
    }
    gamepad = SDL_OpenGamepad(ids[index]);
    SDL_free(ids);
    if (gamepad && SDL_GamepadHasSensor(gamepad, SDL_SENSOR_ACCEL))
        SDL_SetGamepadSensorEnabled(gamepad, SDL_SENSOR_ACCEL, true);
    return gamepad != nullptr;
}

bool SDLWiiRemoteInput::connected() const {
    return gamepad && SDL_GamepadConnected(gamepad);
}

void SDLWiiRemoteInput::update(WiiRemoteState &state) {
    if (!connected()) {
        state.connected = false;
        return;
    }
    state.connected = true;
    state.a = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    state.b = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    state.one = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST);
    state.two = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
    state.plus = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START);
    state.minus = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK);
    state.home = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
    state.dpadUp = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    state.dpadDown = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    state.dpadLeft = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    state.dpadRight =
        SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    state.nunchuk.stickX =
        input::axisToU8(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX));
    state.nunchuk.stickY = input::invertedAxisToU8(
        SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY));
    state.nunchuk.c =
        SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    state.nunchuk.z =
        SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 8192;
    const uint16_t irX = static_cast<uint16_t>(
        input::axisToU8(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX)) *
        1023 / 255);
    const uint16_t irY = static_cast<uint16_t>(
        input::axisToU8(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY)) *
        767 / 255);
    state.ir[0] = {static_cast<uint16_t>(std::max<int>(0, irX - 50)), irY, 4,
                   true};
    state.ir[1] = {static_cast<uint16_t>(std::min<int>(1023, irX + 50)), irY,
                   4, true};
    if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_ACCEL)) {
        float acceleration[3]{};
        if (SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_ACCEL, acceleration,
                                     3)) {
            constexpr float gravity = 9.80665f;
            state.accelX = acceleration[0] / gravity;
            state.accelY = acceleration[1] / gravity;
            state.accelZ = acceleration[2] / gravity;
        }
    }
    SDL_RumbleGamepad(gamepad, state.rumble ? 0xFFFF : 0,
                      state.rumble ? 0xFFFF : 0, 100);
    SDL_SetGamepadLED(gamepad, 0, 0, state.leds ? 255 : 0);
}

void SDLWiiRemoteInput::shutdown() {
    if (!gamepad)
        return;
    SDL_CloseGamepad(gamepad);
    gamepad = nullptr;
}
