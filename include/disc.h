#ifndef REVOLVE_DISK_H
#define REVOLVE_DISK_H

#include "core/executable.h"
#include <array>
#include <cstdint>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
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
    enum class Format { ISO, WBFS, RVZ };

    struct RVZPartitionData {
        uint32_t firstSector;
        uint32_t sectorCount;
        uint32_t groupIndex;
        uint32_t groupCount;
    };

    struct RVZPartition {
        std::array<uint8_t, 16> key;
        std::array<RVZPartitionData, 2> data;
    };

    struct RVZRawData {
        uint64_t offset;
        uint64_t size;
        uint32_t groupIndex;
        uint32_t groupCount;
    };

    struct RVZGroup {
        uint64_t offset;
        uint32_t size;
        uint32_t packedSize;
        bool compressed;
    };

    std::ifstream file;
    uint64_t fileSize = 0;
    uint64_t sourceSize = 0;
    Format format = Format::ISO;

    uint64_t wbfsDiscOffset = 0;
    uint64_t wbfsSectorSize = 0;
    std::vector<uint16_t> wbfsMap;

    uint32_t rvzCompression = 0;
    uint32_t rvzChunkSize = 0;
    std::array<uint8_t, 0x80> rvzDiscHeader{};
    std::vector<RVZPartition> rvzPartitions;
    std::vector<RVZRawData> rvzRawData;
    std::vector<RVZGroup> rvzGroups;
    std::unordered_map<uint32_t, std::vector<uint8_t>> rvzGroupCache;

    std::string gameID;
    std::string title;
    uint8_t discNumber = 0;
    uint8_t revision = 0;
    bool encrypted = true;

    std::array<uint8_t, 16> getPartitionKey(const DiscPartition &partition);
    bool readFile(uint64_t offset, std::span<uint8_t> output);
    bool openWBFS();
    bool openRVZ();
    bool readWBFS(uint64_t offset, std::span<uint8_t> output);
    bool readRVZ(uint64_t offset, std::span<uint8_t> output);
    std::vector<uint8_t> loadRVZGroup(uint32_t index, size_t expectedSize,
                                      bool partitionData);
};

#endif
