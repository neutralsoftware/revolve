#pragma once

#include "input/gamecube.h"
#include <array>

class InputManager {
  public:
    GameCubeControllerState &gameCube(std::size_t index) {
        return gameCubeControllers.at(index);
    }

    const GameCubeControllerState &gameCube(std::size_t index) const {
        return gameCubeControllers.at(index);
    }

    void reset() {
        for (auto &controller : gameCubeControllers) {
            controller = {};
            controller.stickX = 0x80;
            controller.stickY = 0x80;
            controller.cStickX = 0x80;
            controller.cStickY = 0x80;
        }
    }

    inline void update() {
        for (std::size_t i = 0; i < sdlInputs.size(); ++i) {
            sdlInputs[i].update(gameCubeControllers[i]);
        }
    }

  private:
    std::array<GameCubeControllerState, 4> gameCubeControllers{};
    std::array<SDLGameCubeInput, 4> sdlInputs{};
};