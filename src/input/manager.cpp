#include "input/manager.h"

#include <SDL3/SDL.h>

void InputManager::reset() {
    for (auto &controller : gameCubeControllers) {
        controller = {};

        controller.stickX = 0x80;
        controller.stickY = 0x80;
        controller.cStickX = 0x80;
        controller.cStickY = 0x80;
    }

    for (auto &wiimote : wiimotes) {
        wiimote = {};
    }
}

void InputManager::update() {
    updateKeyboard();
    updateGamepads();
}

void InputManager::updateKeyboard() {
    const bool *keys = SDL_GetKeyboardState(nullptr);

    //
    // GameCube controller 0
    //
    auto &gc = gameCubeControllers[0];

    gc.connected = true;

    gc.a = keys[SDL_SCANCODE_X];
    gc.b = keys[SDL_SCANCODE_Z];
    gc.x = keys[SDL_SCANCODE_S];
    gc.y = keys[SDL_SCANCODE_A];

    gc.start = keys[SDL_SCANCODE_RETURN];

    gc.dpadUp = keys[SDL_SCANCODE_UP];
    gc.dpadDown = keys[SDL_SCANCODE_DOWN];
    gc.dpadLeft = keys[SDL_SCANCODE_LEFT];
    gc.dpadRight = keys[SDL_SCANCODE_RIGHT];

    //
    // Wii Remote 0
    //
    auto &wm = wiimotes[0];

    wm.connected = true;

    wm.a = keys[SDL_SCANCODE_SPACE];
    wm.b = keys[SDL_SCANCODE_LSHIFT];

    wm.one = keys[SDL_SCANCODE_1];
    wm.two = keys[SDL_SCANCODE_2];

    wm.plus = keys[SDL_SCANCODE_EQUALS];
    wm.minus = keys[SDL_SCANCODE_MINUS];
    wm.home = keys[SDL_SCANCODE_H];

    wm.dpadUp = keys[SDL_SCANCODE_I];
    wm.dpadDown = keys[SDL_SCANCODE_K];
    wm.dpadLeft = keys[SDL_SCANCODE_J];
    wm.dpadRight = keys[SDL_SCANCODE_L];
}

void InputManager::updateGamepads() {
    for (std::size_t i = 0; i < gameCubeControllers.size(); ++i) {
        auto &gc = gameCubeControllers[i];

        SDLGameCubeInput gcInput;

        if (!gcInput.initialize()) {
            continue;
        }

        gcInput.update(gc);
    }
}
