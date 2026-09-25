#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

struct TitleContent {
    uint32_t id = 0;
    uint16_t index = 0;
    uint16_t type = 0;
    uint64_t size = 0;
    std::array<uint8_t, 20> hash{};
};

struct TitleMetadata {
    uint64_t id = 0;
    uint64_t ios = 0;
    uint16_t bootIndex = 0;
    uint32_t payloadOffset = 0;
    std::vector<TitleContent> contents;
    static std::optional<TitleMetadata> parse(std::span<const uint8_t> data);
};

std::filesystem::path nandTitlePath(uint64_t title);
std::filesystem::path nandTicketPath(uint64_t title);
std::optional<std::filesystem::path>
nandContentPath(uint64_t title, const TitleContent &content);
std::vector<uint8_t> readTitleFile(const std::filesystem::path &path,
                                   uint64_t maximum);
