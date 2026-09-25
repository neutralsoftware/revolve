
#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <span>

struct IRPoint {
    uint16_t x = 0x3FF;
    uint16_t y = 0x3FF;
    uint8_t size = 0;
    bool visible = false;
};

struct NunchukState {
    uint8_t stickX = 0x80;
    uint8_t stickY = 0x80;

    bool c = false;
    bool z = false;

    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 1.0f;
};

struct WiiRemoteState {
    bool connected = false;

    bool a = false;
    bool b = false;

    bool one = false;
    bool two = false;

    bool plus = false;
    bool minus = false;
    bool home = false;

    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;

    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 1.0f;

    std::array<IRPoint, 4> ir{};

    bool nunchukConnected = true;
    NunchukState nunchuk{};
};

class WiiRemoteDevice {
  public:
    explicit WiiRemoteDevice(WiiRemoteState *state) : state(state) {}

    void handleOutputReport(uint8_t reportId, std::span<const uint8_t> payload);

    inline bool hasInputReport() const { return !inputQueue.empty(); }

    std::vector<uint8_t> popInputReport();

    void update();

  private:
    WiiRemoteState *state;

    uint8_t reportMode = 0x30;
    bool continuousReporting = false;

    bool rumble = false;
    bool irEnabled = false;
    bool speakerEnabled = false;

    uint8_t leds = 0;
    uint8_t battery = 0xC0;

    std::array<uint8_t, 256> extensionRegisters{};
    std::array<uint8_t, 0x34> irRegisters{};
    std::array<uint8_t, 0x1700> eeprom{};

    std::array<uint8_t, 256> motionPlusRegisters{};

    std::deque<std::vector<uint8_t>> inputQueue;

    void queueDataReport();
    void queueStatus();
    void queueAck(uint8_t report, uint8_t error = 0);

    void handleReadMemory(std::span<const uint8_t> payload);
    void handleWriteMemory(std::span<const uint8_t> payload);

    std::array<uint8_t, 2> makeButtons() const;
    std::array<uint8_t, 3> makeAccel() const;
    std::array<uint8_t, 6> makeNunchuk() const;

    std::array<uint8_t, 10> makeIRBasic() const;
    std::array<uint8_t, 12> makeIRExtended() const;

    uint8_t readRegister(uint32_t address);
    void writeRegister(uint32_t address, uint8_t value);
};