#include "input/physical_wiimote.h"
#include <algorithm>
#include <array>

PhysicalWiiRemote::~PhysicalWiiRemote() { close(); }

bool PhysicalWiiRemote::open(const std::string &path) {
    close();
    handle = SDL_hid_open_path(path.c_str());
    if (!handle)
        return false;
    devicePath = path;
    return true;
}

void PhysicalWiiRemote::close() {
    if (handle)
        SDL_hid_close(handle);
    handle = nullptr;
    devicePath.clear();
}

bool PhysicalWiiRemote::send(uint8_t report, std::span<const uint8_t> payload) {
    if (!handle || payload.size() > 21)
        return false;
    std::array<unsigned char, 22> data{};
    data[0] = report;
    std::copy(payload.begin(), payload.end(), data.begin() + 1);
    const auto length = payload.size() + 1;
    if (SDL_hid_write(handle, data.data(), length) != static_cast<int>(length)) {
        close();
        return false;
    }
    return true;
}

std::vector<uint8_t> PhysicalWiiRemote::receive() {
    if (!handle)
        return {};
    std::array<unsigned char, 64> data{};
    const int count = SDL_hid_read_timeout(handle, data.data(), data.size(), 0);
    if (count < 0) {
        close();
        return {};
    }
    if (!count)
        return {};
    return {data.begin(), data.begin() + count};
}
