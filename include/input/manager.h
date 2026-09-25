#pragma once

#include "input/gamecube.h"
#include "input/wiimote.h"

#include <array>
#include <cstddef>
#include <memory>

class InputManager {
  public:
    InputManager();

    GameCubeControllerState &gameCube(std::size_t index) {
        return gameCubeControllers.at(index);
    }

    const GameCubeControllerState &gameCube(std::size_t index) const {
        return gameCubeControllers.at(index);
    }

    WiiRemoteState &wiimote(std::size_t index) { return wiimotes.at(index); }

    const WiiRemoteState &wiimote(std::size_t index) const {
        return wiimotes.at(index);
    }

    WiiRemoteDevice &wiimoteDevice(std::size_t index) {
        return *wiimoteDevices.at(index);
    }

    const WiiRemoteDevice &wiimoteDevice(std::size_t index) const {
        return *wiimoteDevices.at(index);
    }

    void reset();
    void update();

  private:
    std::array<GameCubeControllerState, 4> gameCubeControllers{};
    std::array<WiiRemoteState, 4> wiimotes{};

    std::array<SDLGameCubeInput, 4> gamepadInputs{};
    std::array<SDLWiiRemoteInput, 4> wiimoteInputs{};
    std::array<std::unique_ptr<WiiRemoteDevice>, 4> wiimoteDevices{};

    void updateKeyboard();
    void updateGamepads();
};
