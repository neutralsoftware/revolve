#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

void pollNativeBluetooth();
void shutdownNativeBluetooth();
bool nativeBluetoothConnected(std::size_t slot);
bool sendNativeBluetooth(std::size_t slot, uint8_t report, std::span<const uint8_t> payload);
std::vector<uint8_t> receiveNativeBluetooth(std::size_t slot);
