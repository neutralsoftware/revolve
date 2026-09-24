
#include "input/gamecube.h"
#include "SDL3/SDL_keyboard.h"
#include "core/utils.h"

GameCubeReport
input::buildControllerReport(const GameCubeControllerState &state) {
    GameCubeReport report{};

    uint8_t buttons0 = 0;
    uint8_t buttons1 = 0x80;

    if (state.start)
        buttons0 |= 1 << 4;
    if (state.y)
        buttons0 |= 1 << 3;
    if (state.x)
        buttons0 |= 1 << 2;
    if (state.b)
        buttons0 |= 1 << 1;
    if (state.a)
        buttons0 |= 1 << 0;

    if (state.l)
        buttons1 |= 1 << 6;
    if (state.r)
        buttons1 |= 1 << 5;
    if (state.z)
        buttons1 |= 1 << 4;
    if (state.dpadUp)
        buttons1 |= 1 << 3;
    if (state.dpadDown)
        buttons1 |= 1 << 2;
    if (state.dpadRight)
        buttons1 |= 1 << 1;
    if (state.dpadLeft)
        buttons1 |= 1 << 0;

    report.data[0] = buttons0;
    report.data[1] = buttons1;

    report.data[2] = state.stickX;
    report.data[3] = state.stickY;

    report.data[4] = state.cStickX;
    report.data[5] = state.cStickY;

    report.data[6] = state.triggerL;
    report.data[7] = state.triggerR;

    return report;
}

int GameCubeControllerDevice::runCommand(const uint8_t *request,
                                         size_t requestSize, uint8_t *response,
                                         size_t responseCapacity) {
    if (!state || !state->connected) {
        return -1;
    }

    if (requestSize == 0) {
        return 0;
    }

    const auto command = static_cast<GCControllerCommand>(request[0]);

    switch (command) {
    case GCControllerCommand::Status:
    case GCControllerCommand::Reset: {
        if (responseCapacity < 3) {
            return 0;
        }

        response[0] = 0x09;
        response[1] = 0x00;
        response[2] = 0x03;
        return 3;
    }
    case GCControllerCommand::Poll: {
        if (responseCapacity < 8) {
            return 0;
        }

        if (requestSize >= 2) {
            mode = request[1] & 0x07;
        }

        if (requestSize >= 3) {
            rumble = (request[2] & 0x01) != 0;
        }

        auto report = input::buildControllerReport(*state);

        for (size_t i = 0; i < 8; ++i) {
            response[i] = report.data[i];
        }

        return 0;
    }
    case GCControllerCommand::Origin:
    case GCControllerCommand::Recalibrate: {
        if (responseCapacity < 10) {
            return 0;
        }

        auto report = input::buildControllerReport(*state);

        for (size_t i = 0; i < 8; ++i) {
            response[i] = report.data[i];
        }

        response[8] = 0x00;
        response[9] = 0x00;

        return 10;
    }

    default:
        Logger::log("SI", LogLevel::Warning,
                    "Unknown GameCube controller command");

        return 0;
    }
}

bool SDLGameCubeInput::initialize() {
    int count = 0;

    SDL_JoystickID *ids = SDL_GetGamepads(&count);

    if (!ids || count == 0) {
        if (ids)
            SDL_free(ids);

        return false;
    }

    gamepad = SDL_OpenGamepad(ids[0]);

    SDL_free(ids);

    return gamepad != nullptr;
}

void SDLGameCubeInput::update(GameCubeControllerState &state) {
    if (!gamepad) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        state.connected = true;
        state.a = keys[SDL_SCANCODE_X];
        state.b = keys[SDL_SCANCODE_Z];
        state.x = keys[SDL_SCANCODE_S];
        state.y = keys[SDL_SCANCODE_A];
        state.start = keys[SDL_SCANCODE_RETURN];
        state.dpadUp = keys[SDL_SCANCODE_UP];
        state.dpadDown = keys[SDL_SCANCODE_DOWN];
        state.dpadLeft = keys[SDL_SCANCODE_LEFT];
        state.dpadRight = keys[SDL_SCANCODE_RIGHT];

        int sx = 0;
        int sy = 0;

        if (keys[SDL_SCANCODE_D])
            sx += 127;
        if (keys[SDL_SCANCODE_A])
            sx -= 127;

        if (keys[SDL_SCANCODE_W])
            sy += 127;
        if (keys[SDL_SCANCODE_S])
            sy -= 127;

        state.stickX = static_cast<uint8_t>(sx + 128);
        state.stickY = static_cast<uint8_t>(sy + 128);
        return;
    }

    state.connected = true;
    state.a = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
    state.x = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
    state.b = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST);
    state.y = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH);
    state.start = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START);
    state.dpadUp = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    state.dpadDown =
        SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    state.dpadLeft =
        SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    state.dpadRight =
        SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    state.l = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    state.r = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    state.stickX =
        input::axisToU8(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX));
    state.stickY = input::invertedAxisToU8(
        SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY));
    state.cStickX =
        input::axisToU8(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX));
    state.cStickY = input::invertedAxisToU8(
        SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY));
    state.triggerL = input::triggerToU8(
        SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
    state.triggerR = input::triggerToU8(
        SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
}
