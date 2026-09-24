#pragma once

#include "SDL3/SDL_gamepad.h"
#include "input/serial.h"
#include <cstdint>

struct GameCubeControllerState {
    bool connected = false;

    bool a = false;
    bool b = false;
    bool x = false;
    bool y = false;
    bool start = false;

    bool dpadLeft = false;
    bool dpadRight = false;
    bool dpadUp = false;
    bool dpadDown = false;

    bool z = false;
    bool l = false;
    bool r = false;

    uint8_t stickX = 0x80;
    uint8_t stickY = 0x80;

    uint8_t cStickX = 0x80;
    uint8_t cStickY = 0x80;

    uint8_t triggerL = 0;
    uint8_t triggerR = 0;
};

class GameCubeController {
  public:
    GameCubeControllerState &getState() { return state; }

    const GameCubeControllerState &getState() const { return state; }

    void setConnected(bool connected) { state.connected = connected; }

  private:
    GameCubeControllerState state;
};

struct GameCubeReport {
    uint8_t data[8];
};

namespace input {
GameCubeReport buildControllerReport(const GameCubeControllerState &state);

static uint8_t axisToU8(Sint16 value) {
    const int32_t shifted = static_cast<int32_t>(value) + 32768;

    return static_cast<uint8_t>(std::clamp(shifted * 255 / 65535, 0, 255));
}

static uint8_t invertedAxisToU8(Sint16 value) {
    int32_t v = -static_cast<int32_t>(value);
    v = std::clamp(v, -32768, 32767);

    return axisToU8(static_cast<Sint16>(v));
}

static uint8_t triggerToU8(Sint16 value) {
    const int32_t v = std::clamp<int32_t>(value, 0, 32767);
    return static_cast<uint8_t>(v * 255 / 32767);
}
}; // namespace input

enum class GCControllerCommand : uint8_t {
    Status = 0x00,
    Reset = 0xFF,
    Poll = 0x40,
    Origin = 0x41,
    Recalibrate = 0x42
};

class GameCubeControllerDevice final : public SerialInterfaceDevice {
  public:
    explicit GameCubeControllerDevice(GameCubeControllerState *state)
        : state(state) {}

    int runCommand(const uint8_t *request, size_t requestSize,
                   uint8_t *response, size_t responseCapacity) override;

    void sendDirectCommand(uint32_t command, uint8_t poll) override;

  private:
    GameCubeControllerState *state;

    uint8_t mode = 3;
    bool rumble = false;
};

class SDLGameCubeInput {
  public:
    bool initialize();

    void update(GameCubeControllerState &state);

    void shutdown();

  private:
    SDL_Gamepad *gamepad = nullptr;
};