#include "input/native_bluetooth.h"

#ifndef __APPLE__
void pollNativeBluetooth() {}
void shutdownNativeBluetooth() {}
bool nativeBluetoothConnected(std::size_t) { return false; }
bool sendNativeBluetooth(std::size_t, uint8_t, std::span<const uint8_t>) {
    return false;
}
std::vector<uint8_t> receiveNativeBluetooth(std::size_t) { return {}; }
#endif
