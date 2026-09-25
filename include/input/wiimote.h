
#pragma once

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
};