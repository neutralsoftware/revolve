#include "input/manager.h"
#include "device.h"
#include "input/native_bluetooth.h"

#include <SDL3/SDL.h>
#include <algorithm>

InputManager::InputManager() {
    for (std::size_t i = 0; i < wiimotes.size(); ++i) {
        wiimoteDevices[i] = std::make_unique<WiiRemoteDevice>(&wiimotes[i]);
    }

    reset();
}

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
    for (auto &gc : gameCubeControllers) {
        const bool rumble = gc.rumble;
        gc = {};
        gc.rumble = rumble;
        gc.stickX = 0x80;
        gc.stickY = 0x80;
        gc.cStickX = 0x80;
        gc.cStickY = 0x80;
    }
    for (auto &wiimote : wiimotes) {
        const bool rumble = wiimote.rumble;
        const uint8_t leds = wiimote.leds;
        wiimote = {};
        wiimote.rumble = rumble;
        wiimote.leds = leds;
    }
    updateGamepads();
    updateKeyboard();
    updatePhysical();
}

void InputManager::updateKeyboard() {
    const bool *keys = SDL_GetKeyboardState(nullptr);

    //
    // GameCube controller 0
    //
    auto &gc = gameCubeControllers[0];

    gc.connected = true;

    gc.a |= keys[SDL_SCANCODE_X];
    gc.b |= keys[SDL_SCANCODE_Z];
    gc.x |= keys[SDL_SCANCODE_S];
    gc.y |= keys[SDL_SCANCODE_A];

    gc.start |= keys[SDL_SCANCODE_RETURN];
    gc.z |= keys[SDL_SCANCODE_C];
    gc.l |= keys[SDL_SCANCODE_Q];
    gc.r |= keys[SDL_SCANCODE_E];

    gc.dpadUp |= keys[SDL_SCANCODE_UP];
    gc.dpadDown |= keys[SDL_SCANCODE_DOWN];
    gc.dpadLeft |= keys[SDL_SCANCODE_LEFT];
    gc.dpadRight |= keys[SDL_SCANCODE_RIGHT];

    //
    // Wii Remote 0
    //
    auto &wm = wiimotes[0];

    wm.connected = true;

    wm.a |= keys[SDL_SCANCODE_SPACE];
    wm.b |= keys[SDL_SCANCODE_LSHIFT];

    wm.one |= keys[SDL_SCANCODE_1];
    wm.two |= keys[SDL_SCANCODE_2];

    wm.plus |= keys[SDL_SCANCODE_EQUALS];
    wm.minus |= keys[SDL_SCANCODE_MINUS];
    wm.home |= keys[SDL_SCANCODE_H];

    wm.dpadUp |= keys[SDL_SCANCODE_UP];
    wm.dpadDown |= keys[SDL_SCANCODE_DOWN];
    wm.dpadLeft |= keys[SDL_SCANCODE_LEFT];
    wm.dpadRight |= keys[SDL_SCANCODE_RIGHT];

    int nx = wm.nunchuk.stickX;
    int ny = wm.nunchuk.stickY;

    if (keys[SDL_SCANCODE_A])
        nx = 30;
    if (keys[SDL_SCANCODE_D])
        nx = 225;

    if (keys[SDL_SCANCODE_W])
        ny = 225;
    if (keys[SDL_SCANCODE_S])
        ny = 30;

    wm.nunchuk.stickX = static_cast<uint8_t>(nx);
    wm.nunchuk.stickY = static_cast<uint8_t>(ny);

    wm.nunchuk.c |= keys[SDL_SCANCODE_C];
    wm.nunchuk.z |= keys[SDL_SCANCODE_Z];

    float mouseX = 0;
    float mouseY = 0;

    SDL_GetMouseState(&mouseX, &mouseY);
    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSize(Device::globalDevice->gx.renderer->window, &windowWidth,
                      &windowHeight);

    if (windowWidth <= 0 || windowHeight <= 0)
        return;

    const float px = mouseX / float(windowWidth);
    const float py = mouseY / float(windowHeight);

    uint16_t irX = static_cast<uint16_t>(std::clamp(px, 0.0f, 1.0f) * 1023.0f);

    uint16_t irY = static_cast<uint16_t>(std::clamp(py, 0.0f, 1.0f) * 767.0f);

    constexpr int separation = 100;

    wm.ir[0] = {
        static_cast<uint16_t>(std::clamp<int>(irX - separation / 2, 0, 1023)),
        irY, 4, true};

    wm.ir[1] = {
        static_cast<uint16_t>(std::clamp<int>(irX + separation / 2, 0, 1023)),
        irY, 4, true};

    wm.ir[2].visible = false;
    wm.ir[3].visible = false;
}

void InputManager::updateGamepads() {
    for (std::size_t i = 0; i < gameCubeControllers.size(); ++i) {
        auto &gc = gameCubeControllers[i];
        auto &input = gamepadInputs[i];

        if (!input.connected() && !input.initialize(i)) {
            if (i != 0)
                gc.connected = false;
            continue;
        }

        input.update(gc);

        auto &wiimoteInput = wiimoteInputs[i];
        if (!wiimoteInput.connected() && !wiimoteInput.initialize(i))
            continue;
        wiimoteInput.update(wiimotes[i]);
    }
}

void InputManager::updatePhysical() {
    const uint64_t now = SDL_GetTicks();
    if (bluetoothDiscovery)
        pollNativeBluetooth();
    if (!bluetoothDiscovery && now >= nextPhysicalScan) {
        nextPhysicalScan = now + 2000;
        auto *devices = SDL_hid_enumerate(0x057E, 0);
        for (auto *candidate = devices; candidate;
             candidate = candidate->next) {
            if (!candidate->path || (candidate->product_id != 0x0306 &&
                                     candidate->product_id != 0x0330))
                continue;
            const bool assigned =
                std::any_of(physicalWiimotes.begin(), physicalWiimotes.end(),
                            [&](const auto &remote) {
                                return remote.path() == candidate->path;
                            });
            if (assigned)
                continue;
            for (auto &remote : physicalWiimotes) {
                if (!remote.connected()) {
                    remote.open(candidate->path);
                    break;
                }
            }
        }
        SDL_hid_free_enumeration(devices);
    }
    for (std::size_t i = 0; i < physicalWiimotes.size(); ++i) {
        auto &remote = physicalWiimotes[i];
        auto &device = *wiimoteDevices[i];
        if (bluetoothDiscovery) {
            remote.useNative(i);
            device.requirePhysical();
        }
        if (remote.connected())
            device.requirePhysical();
        device.attachPhysical(remote.connected() ? &remote : nullptr);
        device.pollPhysical();
        if (remote.connected())
            wiimotes[i].connected = true;
    }
}

InputManager::~InputManager() {
    if (bluetoothDiscovery)
        shutdownNativeBluetooth();
}
