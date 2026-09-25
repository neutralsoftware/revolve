#pragma once

#include <SDL3/SDL_hidapi.h>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

class PhysicalWiiRemote {
  public:
    ~PhysicalWiiRemote();
    PhysicalWiiRemote() = default;
    PhysicalWiiRemote(const PhysicalWiiRemote &) = delete;
    PhysicalWiiRemote &operator=(const PhysicalWiiRemote &) = delete;

    bool open(const std::string &path);
    void close();
    bool connected() const;
    void useNative(std::size_t slot);
    const std::string &path() const { return devicePath; }
    bool send(uint8_t report, std::span<const uint8_t> payload);
    std::vector<uint8_t> receive();

  private:
    int nativeSlot = -1;
    SDL_hid_device *handle = nullptr;
    std::string devicePath;
};
