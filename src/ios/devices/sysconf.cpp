#include "ios/ios.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>
#include <string_view>

void initializeSystemConfiguration(const std::filesystem::path &root) {
    const auto config = root / "shared2/sys/SYSCONF";
    if (!std::filesystem::exists(config)) {
        struct Entry {
            std::string_view name;
            uint8_t type;
            std::vector<uint8_t> bytes;
        };
        const std::vector<Entry> entries{
            {"IPL.LNG", 3, {1}},
            {"IPL.SND", 3, {1}},
            {"IPL.AR", 3, {1}},
            {"IPL.PGS", 3, {1}},
            {"IPL.E60", 3, {1}},
            {"IPL.SSV", 3, {0}},
            {"IPL.CB", 5, {0, 0, 0, 0}},
            {"IPL.DH", 3, {0}},
            {"IPL.IDL", 2, {0, 0}},
            {"IPL.NIK", 2, {0,   'R', 0,   'e', 0, 'v', 0, 'o', 0, 'l', 0,
                            'v', 0,   'e', 0,   0, 0,   0, 0,   0, 7}},
            {"BT.BAR", 3, {1}},
            {"BT.SENS", 5, {0, 0, 0, 3}},
            {"BT.SPKV", 3, {0x58}},
            {"BT.MOT", 3, {1}},
            {"BT.DINF", 1, std::vector<uint8_t>(0x461)},
            {"BT.CDIF", 1, std::vector<uint8_t>(0x205)},
        };
        std::array<uint8_t, 0x4000> data{};
        const auto put16 = [&](size_t at, uint16_t value) {
            data.at(at) = value >> 8;
            data.at(at + 1) = value;
        };
        std::copy_n("SCv0", 4, data.begin());
        std::copy_n("SCed", 4, data.end() - 4);
        put16(4, entries.size());
        size_t cursor = 6 + (entries.size() + 1) * 2;
        size_t index = 0;
        for (const auto &entry : entries) {
            put16(6 + index++ * 2, cursor);
            data.at(cursor++) = (entry.type << 5) | (entry.name.size() - 1);
            for (char c : entry.name)
                data.at(cursor++) = c;
            if (entry.type == 1) {
                put16(cursor, entry.bytes.size() - 1);
                cursor += 2;
            } else if (entry.type == 2) {
                data.at(cursor++) = entry.bytes.size() - 1;
            }
            for (auto byte : entry.bytes)
                data.at(cursor++) = byte;
        }
        put16(6 + entries.size() * 2, cursor);
        std::ofstream file(config, std::ios::binary);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        if (!file)
            throw std::runtime_error("Cannot initialize SYSCONF");
    }
    const auto settings = root / "title/00000001/00000002/data/setting.txt";
    if (!std::filesystem::exists(settings)) {
        std::filesystem::create_directories(settings.parent_path());
        std::array<uint8_t, 256> data{};
        constexpr std::string_view text =
            "AREA=EUR\r\nMODEL=RVL-001(EUR)\r\nDVD=0\r\nMPCH=0x7FFE\r\nCODE="
            "LEH\r\nSERNO=000000000\r\nVIDEO=PAL\r\nGAME=EU\r\n";
        std::copy(text.begin(), text.end(), data.begin());
        uint32_t key = 0x73B5DBFA;
        for (auto &byte : data) {
            byte ^= static_cast<uint8_t>(key);
            key = (key << 1) | (key >> 31);
        }
        std::ofstream file(settings, std::ios::binary);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        if (!file)
            throw std::runtime_error("Cannot initialize Wii settings");
    }
}
