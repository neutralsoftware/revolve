#pragma once

#include "input/gamecube.h"
#include "input/wiimote.h"
#include <array>

class InputManager {
  public:
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

    void reset();
    void update();

  private:
    std::array<GameCubeControllerState, 4> gameCubeControllers{};
    std::array<WiiRemoteState, 4> wiimotes{};

    void updateKeyboard();
    void updateGamepads();
};