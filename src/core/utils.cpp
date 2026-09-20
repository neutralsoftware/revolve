#include "core/utils.h"

#include <chrono>
#include <iostream>
#include <stdexcept>

BigEndianStream::BigEndianStream(const std::string &path)
    : file(path, std::ios::binary) {
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }
}

void BigEndianStream::moveTo(uint32_t offset) {
    file.seekg(offset, std::ios::beg);

    if (!file) {
        throw std::runtime_error("Failed to seek");
    }
}

void BigEndianStream::moveBy(int32_t offset) {
    file.seekg(offset, std::ios::cur);

    if (!file) {
        throw std::runtime_error("Failed to seek");
    }
}

uint8_t BigEndianStream::readByte() {
    uint8_t value;

    file.read(reinterpret_cast<char *>(&value), 1);

    if (!file) {
        throw std::runtime_error("Unexpected end of file");
    }

    return value;
}

uint16_t BigEndianStream::readShort() {
    uint8_t bytes[2];

    file.read(reinterpret_cast<char *>(bytes), 2);

    if (!file) {
        throw std::runtime_error("Unexpected end of file");
    }

    return (static_cast<uint16_t>(bytes[0]) << 8) |
           static_cast<uint16_t>(bytes[1]);
}

uint32_t BigEndianStream::readInt() {
    uint8_t bytes[4];

    file.read(reinterpret_cast<char *>(bytes), 4);

    if (!file) {
        throw std::runtime_error("Unexpected end of file");
    }

    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) |
           static_cast<uint32_t>(bytes[3]);
}

std::vector<uint8_t> BigEndianStream::readNBytes(size_t n) {
    std::vector<uint8_t> bytes(n);

    file.read(reinterpret_cast<char *>(bytes.data()),
              static_cast<std::streamsize>(n));

    if (!file) {
        throw std::runtime_error("Unexpected end of file");
    }

    return bytes;
}

std::string getTime() {
    const auto now = std::chrono::system_clock::now();

    return std::format("{:%H:%M:%S}",
                       std::chrono::floor<std::chrono::seconds>(now));
}

void Logger::log(std::string system, LogLevel level,
                 const std::string &message) {
    std::cout << "[" << system << "] @ " << getTime() << "["
              << static_cast<int>(level) << "] " << message << std::endl;
}

void Logger::logObject(const Loggable &object, LogLevel level) {
    log(object.getLogSystem(), level, object.log());
}
