#ifndef REVOLVE_DISK_H
#define REVOLVE_DISK_H

#include "core/executable.h"
#include <array>
#include <cstdint>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <vector>

struct DiscPartition {
    uint64_t offset;
    uint32_t type;
    uint64_t dataOffset;
    uint64_t dataSize;
};

class DiscImage {
  public:
    bool open(const std::string &path);

    inline bool isOpen() const { return file.is_open(); }
    inline uint64_t size() const { return fileSize; }

    bool readRaw(uint64_t offset, std::span<uint8_t> output);

    uint32_t readBE32(uint64_t offset);

    std::vector<DiscPartition> getPartitions();
    std::optional<DiscPartition> getDataPartition();
    bool readPartition(const DiscPartition &partition, uint64_t offset,
                       std::span<uint8_t> output);
    Executable getExecutable();
    std::string log();

    const std::string &getGameID() const { return gameID; }
    const std::string &getTitle() const { return title; }
    uint8_t getDiscNumber() const { return discNumber; }
    uint8_t getRevision() const { return revision; }
    bool isEncrypted() const { return encrypted; }

  private:
    std::ifstream file;
    uint64_t fileSize = 0;

    std::string gameID;
    std::string title;
    uint8_t discNumber = 0;
    uint8_t revision = 0;
    bool encrypted = true;

    std::array<uint8_t, 16>
    getPartitionKey(const DiscPartition &partition);
};

#endif
