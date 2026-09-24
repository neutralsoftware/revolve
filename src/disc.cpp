#include "disc.h"
#include "device.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <zstd.h>

namespace {
constexpr uint64_t CLUSTER_SIZE = 0x8000;
constexpr uint64_t CLUSTER_HEADER_SIZE = 0x400;
constexpr uint64_t CLUSTER_DATA_SIZE = 0x7C00;

const std::array<std::array<uint8_t, 16>, 2> COMMON_KEYS = {{
    {0xEB, 0xE4, 0x2A, 0x22, 0x5E, 0x85, 0x93, 0xE4, 0x48, 0xD9, 0xC5, 0x45,
     0x73, 0x81, 0xAA, 0xF7},
    {0x63, 0xB8, 0x2B, 0xB4, 0xF4, 0x61, 0x4E, 0x2E, 0x13, 0xF2, 0xFE, 0xFB,
     0xBA, 0x4C, 0x9B, 0x7E},
}};

uint32_t readBE32(std::span<const uint8_t> data, size_t offset) {
    if (offset > data.size() || data.size() - offset < 4)
        throw std::runtime_error("Invalid big-endian read");
    return (static_cast<uint32_t>(data[offset]) << 24) |
           (static_cast<uint32_t>(data[offset + 1]) << 16) |
           (static_cast<uint32_t>(data[offset + 2]) << 8) |
           static_cast<uint32_t>(data[offset + 3]);
}

uint64_t readBE64(std::span<const uint8_t> data, size_t offset) {
    return (static_cast<uint64_t>(readBE32(data, offset)) << 32) |
           readBE32(data, offset + 4);
}

std::vector<uint8_t> decompress(std::span<const uint8_t> input, size_t expected,
                                uint32_t compression) {
    if (compression == 0) {
        if (input.size() != expected)
            throw std::runtime_error("Invalid uncompressed RVZ block size");
        return {input.begin(), input.end()};
    }
    if (compression != 5)
        throw std::runtime_error("Unsupported RVZ compression method " +
                                 std::to_string(compression));
    std::vector<uint8_t> output(expected);
    size_t size = ZSTD_decompress(output.data(), output.size(), input.data(),
                                  input.size());
    if (ZSTD_isError(size) || size != expected)
        throw std::runtime_error("RVZ Zstandard decompression failed");
    return output;
}

void decryptCBC(std::span<const uint8_t, 16> key,
                std::span<const uint8_t, 16> iv, std::span<const uint8_t> input,
                std::span<uint8_t> output) {
    if (input.size() != output.size() || input.size() % 16 != 0)
        throw std::runtime_error("Invalid AES-CBC buffer size");

    EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
    if (!context)
        throw std::runtime_error("Failed to create AES context");

    int written = 0;
    int finalWritten = 0;
    bool success =
        EVP_DecryptInit_ex(context, EVP_aes_128_cbc(), nullptr, key.data(),
                           iv.data()) == 1 &&
        EVP_CIPHER_CTX_set_padding(context, 0) == 1 &&
        EVP_DecryptUpdate(context, output.data(), &written, input.data(),
                          static_cast<int>(input.size())) == 1 &&
        EVP_DecryptFinal_ex(context, output.data() + written, &finalWritten) ==
            1;
    EVP_CIPHER_CTX_free(context);

    if (!success ||
        static_cast<size_t>(written + finalWritten) != output.size())
        throw std::runtime_error("AES-CBC decryption failed");
}

std::string partitionType(uint32_t type) {
    if (type == 0)
        return "Data";
    if (type == 1)
        return "Update";
    if (type == 2)
        return "Channel";
    return "Type " + std::to_string(type);
}
}

bool DiscImage::open(const std::string &path) {
    if (file.is_open())
        file.close();
    file.clear();
    file.open(path, std::ios::binary);
    gameID.clear();
    title.clear();
    fileSize = 0;
    sourceSize = 0;
    format = Format::ISO;
    wbfsMap.clear();
    rvzPartitions.clear();
    rvzRawData.clear();
    rvzGroups.clear();
    rvzGroupCache.clear();

    if (!file)
        return false;

    try {
        sourceSize = std::filesystem::file_size(path);
        fileSize = sourceSize;
        std::array<uint8_t, 4> magic{};
        if (!readFile(0, magic))
            throw std::runtime_error("Invalid disc image");
        if (std::memcmp(magic.data(), "WBFS", 4) == 0) {
            format = Format::WBFS;
            if (!openWBFS())
                throw std::runtime_error("Invalid WBFS image");
        } else if (magic == std::array<uint8_t, 4>{'R', 'V', 'Z', 1}) {
            format = Format::RVZ;
            if (!openRVZ())
                throw std::runtime_error("Invalid RVZ image");
        }
        if (fileSize < 0x40020 || readBE32(0x18) != 0x5D1C9EA3)
            throw std::runtime_error("Invalid Wii disc image");

        std::array<uint8_t, 8> identity{};
        if (!readRaw(0, identity))
            throw std::runtime_error("Invalid Wii disc identity");
        gameID.assign(reinterpret_cast<const char *>(identity.data()), 6);
        discNumber = identity[6];
        revision = identity[7];

        std::array<uint8_t, 66> header{};
        if (!readRaw(0x20, header))
            throw std::runtime_error("Invalid Wii disc header");
        size_t titleLength = 0;
        while (titleLength < 64 && header[titleLength] != 0)
            ++titleLength;
        title.assign(reinterpret_cast<const char *>(header.data()),
                     titleLength);
        encrypted = header[65] == 0;
        getPartitions();
        return true;
    } catch (...) {
        file.close();
        fileSize = 0;
        gameID.clear();
        title.clear();
        return false;
    }
}

bool DiscImage::readFile(uint64_t offset, std::span<uint8_t> output) {
    if (!file || offset > sourceSize || output.size() > sourceSize - offset)
        return false;

    file.clear();
    file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!file)
        return false;
    file.read(reinterpret_cast<char *>(output.data()),
              static_cast<std::streamsize>(output.size()));
    return static_cast<size_t>(file.gcount()) == output.size();
}

bool DiscImage::readRaw(uint64_t offset, std::span<uint8_t> output) {
    if (offset > fileSize || output.size() > fileSize - offset)
        return false;
    if (format == Format::WBFS)
        return readWBFS(offset, output);
    if (format == Format::RVZ)
        return readRVZ(offset, output);
    return readFile(offset, output);
}

bool DiscImage::openWBFS() {
    std::array<uint8_t, 12> header{};
    if (!readFile(0, header))
        return false;
    uint8_t hdShift = header[8];
    uint8_t wbfsShift = header[9];
    if (hdShift < 9 || hdShift > 31 || wbfsShift < hdShift || wbfsShift > 31)
        return false;
    uint64_t hdSectorSize = uint64_t{1} << hdShift;
    wbfsSectorSize = uint64_t{1} << wbfsShift;
    std::vector<uint8_t> discTable(static_cast<size_t>(hdSectorSize - 12));
    if (!readFile(12, discTable))
        return false;
    auto entry = std::find_if(discTable.begin(), discTable.end(),
                              [](uint8_t value) { return value != 0; });
    if (entry == discTable.end())
        return false;
    wbfsDiscOffset = static_cast<uint64_t>(*entry) * hdSectorSize;
    constexpr uint64_t WII_DISC_SIZE = 0x118240000;
    size_t blockCount = static_cast<size_t>(
        (WII_DISC_SIZE + wbfsSectorSize - 1) / wbfsSectorSize);
    std::vector<uint8_t> map(blockCount * 2);
    if (!readFile(wbfsDiscOffset + 0x100, map))
        return false;
    wbfsMap.resize(blockCount);
    for (size_t index = 0; index < blockCount; ++index)
        wbfsMap[index] =
            static_cast<uint16_t>(map[index * 2] << 8) | map[index * 2 + 1];
    fileSize = WII_DISC_SIZE;
    return true;
}

bool DiscImage::readWBFS(uint64_t offset, std::span<uint8_t> output) {
    size_t copied = 0;
    while (copied < output.size()) {
        uint64_t position = offset + copied;
        uint64_t block = position / wbfsSectorSize;
        size_t inBlock = static_cast<size_t>(position % wbfsSectorSize);
        size_t amount = std::min(output.size() - copied,
                                 static_cast<size_t>(wbfsSectorSize) - inBlock);
        if (block >= wbfsMap.size())
            return false;
        if (wbfsMap[block] == 0) {
            std::fill_n(output.data() + copied, amount, 0);
        } else if (!readFile(static_cast<uint64_t>(wbfsMap[block]) *
                                     wbfsSectorSize +
                                 inBlock,
                             output.subspan(copied, amount))) {
            return false;
        }
        copied += amount;
    }
    return true;
}

bool DiscImage::openRVZ() {
    std::array<uint8_t, 0x48> fileHeader{};
    std::array<uint8_t, 0xDC> discHeader{};
    if (!readFile(0, fileHeader) || !readFile(0x48, discHeader))
        return false;
    fileSize = ::readBE64(fileHeader, 0x24);
    if (::readBE32(discHeader, 0) != 2)
        return false;
    rvzCompression = ::readBE32(discHeader, 4);
    rvzChunkSize = ::readBE32(discHeader, 0xC);
    if (rvzChunkSize < 0x8000 || rvzChunkSize % 0x8000 != 0)
        return false;
    std::copy_n(discHeader.data() + 0x10, rvzDiscHeader.size(),
                rvzDiscHeader.begin());

    uint32_t partitionCount = ::readBE32(discHeader, 0x90);
    uint32_t partitionSize = ::readBE32(discHeader, 0x94);
    uint64_t partitionOffset = ::readBE64(discHeader, 0x98);
    if (partitionSize < 48 || partitionCount > 1024)
        return false;
    std::vector<uint8_t> partitionTable(static_cast<size_t>(partitionCount) *
                                        partitionSize);
    if (!readFile(partitionOffset, partitionTable))
        return false;
    for (uint32_t index = 0; index < partitionCount; ++index) {
        std::span<const uint8_t> entry(
            partitionTable.data() + index * partitionSize, partitionSize);
        RVZPartition partition{};
        std::copy_n(entry.begin(), 16, partition.key.begin());
        for (uint32_t part = 0; part < 2; ++part) {
            size_t base = 16 + part * 16;
            partition.data[part] = {
                ::readBE32(entry, base), ::readBE32(entry, base + 4),
                ::readBE32(entry, base + 8), ::readBE32(entry, base + 12)};
        }
        rvzPartitions.push_back(partition);
    }

    uint32_t rawCount = ::readBE32(discHeader, 0xB4);
    uint64_t rawOffset = ::readBE64(discHeader, 0xB8);
    uint32_t rawStoredSize = ::readBE32(discHeader, 0xC0);
    std::vector<uint8_t> rawStored(rawStoredSize);
    if (!readFile(rawOffset, rawStored))
        return false;
    auto rawTable = decompress(rawStored, static_cast<size_t>(rawCount) * 24,
                               rvzCompression);
    for (uint32_t index = 0; index < rawCount; ++index) {
        size_t base = index * 24;
        rvzRawData.push_back(
            {::readBE64(rawTable, base), ::readBE64(rawTable, base + 8),
             ::readBE32(rawTable, base + 16), ::readBE32(rawTable, base + 20)});
    }

    uint32_t groupCount = ::readBE32(discHeader, 0xC4);
    uint64_t groupOffset = ::readBE64(discHeader, 0xC8);
    uint32_t groupStoredSize = ::readBE32(discHeader, 0xD0);
    std::vector<uint8_t> groupStored(groupStoredSize);
    if (!readFile(groupOffset, groupStored))
        return false;
    auto groupTable = decompress(
        groupStored, static_cast<size_t>(groupCount) * 12, rvzCompression);
    for (uint32_t index = 0; index < groupCount; ++index) {
        size_t base = index * 12;
        uint32_t size = ::readBE32(groupTable, base + 4);
        rvzGroups.push_back(
            {static_cast<uint64_t>(::readBE32(groupTable, base)) << 2,
             size & 0x7FFFFFFFu, ::readBE32(groupTable, base + 8),
             (size & 0x80000000u) != 0});
    }
    return true;
}

std::vector<uint8_t> DiscImage::loadRVZGroup(uint32_t index,
                                             size_t expectedSize,
                                             bool partitionData) {
    if (auto found = rvzGroupCache.find(index); found != rvzGroupCache.end())
        return found->second;
    if (index >= rvzGroups.size())
        throw std::runtime_error("Invalid RVZ group index");
    const auto &group = rvzGroups[index];
    if (group.size == 0) {
        std::vector<uint8_t> zeroes(expectedSize);
        rvzGroupCache[index] = zeroes;
        return zeroes;
    }

    std::vector<uint8_t> stored(group.size);
    if (!readFile(group.offset, stored))
        throw std::runtime_error("Failed to read RVZ group");
    std::vector<uint8_t> decoded;
    if (group.compressed) {
        size_t decompressedSize = group.packedSize;
        if (decompressedSize == 0) {
            unsigned long long frameSize =
                ZSTD_getFrameContentSize(stored.data(), stored.size());
            if (frameSize == ZSTD_CONTENTSIZE_ERROR ||
                frameSize == ZSTD_CONTENTSIZE_UNKNOWN || frameSize > SIZE_MAX)
                throw std::runtime_error("Invalid RVZ Zstandard frame");
            decompressedSize = static_cast<size_t>(frameSize);
        }
        decoded = decompress(stored, decompressedSize, rvzCompression);
    } else {
        decoded = std::move(stored);
    }

    if (group.packedSize != 0) {
        std::vector<uint8_t> unpacked;
        size_t position = 0;
        while (position < decoded.size()) {
            uint32_t size = ::readBE32(decoded, position);
            position += 4;
            bool generated = size & 0x80000000u;
            size &= 0x7FFFFFFFu;
            if (!generated) {
                if (size > decoded.size() - position)
                    throw std::runtime_error("Invalid RVZ packed data");
                unpacked.insert(unpacked.end(), decoded.begin() + position,
                                decoded.begin() + position + size);
                position += size;
                continue;
            }
            if (decoded.size() - position < 68)
                throw std::runtime_error("Invalid RVZ PRNG seed");
            std::array<uint32_t, 521> state{};
            for (size_t word = 0; word < 17; ++word)
                state[word] = ::readBE32(decoded, position + word * 4);
            position += 68;
            for (size_t word = 17; word < state.size(); ++word)
                state[word] = (state[word - 17] << 23) ^
                              (state[word - 16] >> 9) ^ state[word - 1];
            auto advance = [&] {
                for (size_t word = 0; word < 32; ++word)
                    state[word] ^= state[word + state.size() - 32];
                for (size_t word = 32; word < state.size(); ++word)
                    state[word] ^= state[word - 32];
            };
            for (uint32_t iteration = 0; iteration < 4; ++iteration)
                advance();
            size_t generatedBytes = 0;
            size_t word = 0;
            while (generatedBytes < size) {
                if (word == state.size()) {
                    advance();
                    word = 0;
                }
                std::array<uint8_t, 4> bytes = {
                    static_cast<uint8_t>(state[word] >> 24),
                    static_cast<uint8_t>(state[word] >> 18),
                    static_cast<uint8_t>(state[word] >> 8),
                    static_cast<uint8_t>(state[word])};
                size_t amount = std::min<size_t>(4, size - generatedBytes);
                unpacked.insert(unpacked.end(), bytes.begin(),
                                bytes.begin() + amount);
                generatedBytes += amount;
                ++word;
            }
        }
        decoded = std::move(unpacked);
    }

    if (partitionData) {
        size_t position = 0;
        uint32_t lists = std::max<uint32_t>(1, rvzChunkSize / 0x200000);
        for (uint32_t list = 0; list < lists; ++list) {
            if (decoded.size() - position < 2)
                throw std::runtime_error("Invalid RVZ exception list");
            uint16_t count = static_cast<uint16_t>(decoded[position] << 8) |
                             decoded[position + 1];
            position += 2;
            uint64_t exceptionBytes = static_cast<uint64_t>(count) * 22;
            if (exceptionBytes > decoded.size() - position)
                throw std::runtime_error("Invalid RVZ exception data");
            position += static_cast<size_t>(exceptionBytes);
        }
        if (!group.compressed)
            position = (position + 3) & ~size_t{3};
        decoded.erase(decoded.begin(), decoded.begin() + position);
    }
    if (decoded.size() < expectedSize)
        decoded.resize(expectedSize, 0);
    else if (decoded.size() > expectedSize)
        decoded.resize(expectedSize);
    rvzGroupCache[index] = decoded;
    return decoded;
}

bool DiscImage::readRVZ(uint64_t offset, std::span<uint8_t> output) {
    size_t copied = 0;
    while (copied < output.size()) {
        uint64_t position = offset + copied;
        if (position < rvzDiscHeader.size()) {
            size_t amount =
                std::min(output.size() - copied,
                         rvzDiscHeader.size() - static_cast<size_t>(position));
            std::copy_n(rvzDiscHeader.data() + position, amount,
                        output.data() + copied);
            copied += amount;
            continue;
        }
        auto raw = std::find_if(rvzRawData.begin(), rvzRawData.end(),
                                [&](const RVZRawData &entry) {
                                    return position >= entry.offset &&
                                           position < entry.offset + entry.size;
                                });
        if (raw == rvzRawData.end()) {
            std::fill(output.begin() + copied, output.end(), 0);
            return true;
        }
        uint64_t relative = position - raw->offset;
        uint32_t groupWithin = static_cast<uint32_t>(relative / rvzChunkSize);
        if (groupWithin >= raw->groupCount)
            return false;
        uint64_t groupStart = static_cast<uint64_t>(groupWithin) * rvzChunkSize;
        size_t expected = static_cast<size_t>(
            std::min<uint64_t>(rvzChunkSize, raw->size - groupStart));
        auto data =
            loadRVZGroup(raw->groupIndex + groupWithin, expected, false);
        size_t inGroup = static_cast<size_t>(relative - groupStart);
        size_t amount = std::min(output.size() - copied, data.size() - inGroup);
        std::copy_n(data.data() + inGroup, amount, output.data() + copied);
        copied += amount;
    }
    return true;
}

uint32_t DiscImage::readBE32(uint64_t offset) {
    std::array<uint8_t, 4> data{};
    if (!readRaw(offset, data))
        throw std::runtime_error("Disc read outside image");
    return ::readBE32(data, 0);
}

std::vector<DiscPartition> DiscImage::getPartitions() {
    if (!isOpen())
        throw std::runtime_error("Disc image is not open");

    std::vector<DiscPartition> partitions;
    for (uint32_t group = 0; group < 4; ++group) {
        uint64_t descriptor = 0x40000 + static_cast<uint64_t>(group) * 8;
        uint32_t count = readBE32(descriptor);
        uint64_t tableOffset = static_cast<uint64_t>(readBE32(descriptor + 4))
                               << 2;
        if (count == 0)
            continue;
        if (count > 1024 || tableOffset > fileSize ||
            static_cast<uint64_t>(count) * 8 > fileSize - tableOffset)
            throw std::runtime_error("Invalid Wii partition table");

        for (uint32_t index = 0; index < count; ++index) {
            uint64_t entry = tableOffset + static_cast<uint64_t>(index) * 8;
            DiscPartition partition{};
            partition.offset = static_cast<uint64_t>(readBE32(entry)) << 2;
            partition.type = readBE32(entry + 4);
            if (partition.offset > fileSize ||
                fileSize - partition.offset < 0x2C0)
                throw std::runtime_error("Invalid Wii partition offset");
            partition.dataOffset =
                static_cast<uint64_t>(readBE32(partition.offset + 0x2B8)) << 2;
            partition.dataSize =
                static_cast<uint64_t>(readBE32(partition.offset + 0x2BC)) << 2;
            if (partition.dataOffset > fileSize - partition.offset ||
                partition.dataSize >
                    fileSize - partition.offset - partition.dataOffset)
                throw std::runtime_error("Invalid Wii partition data range");
            partitions.push_back(partition);
        }
    }
    return partitions;
}

std::optional<DiscPartition> DiscImage::getDataPartition() {
    for (const auto &partition : getPartitions()) {
        if (partition.type == 0)
            return partition;
    }
    return std::nullopt;
}

std::array<uint8_t, 16>
DiscImage::getPartitionKey(const DiscPartition &partition) {
    std::array<uint8_t, 16> encryptedKey{};
    std::array<uint8_t, 16> titleKey{};
    std::array<uint8_t, 16> iv{};
    std::array<uint8_t, 8> titleID{};
    std::array<uint8_t, 1> commonKeyIndex{};

    if (!readRaw(partition.offset + 0x1BF, encryptedKey) ||
        !readRaw(partition.offset + 0x1DC, titleID) ||
        !readRaw(partition.offset + 0x1F1, commonKeyIndex))
        throw std::runtime_error("Invalid Wii partition ticket");
    if (commonKeyIndex[0] >= COMMON_KEYS.size())
        throw std::runtime_error("Unsupported Wii common key index " +
                                 std::to_string(commonKeyIndex[0]));

    std::copy(titleID.begin(), titleID.end(), iv.begin());
    decryptCBC(COMMON_KEYS[commonKeyIndex[0]], iv, encryptedKey, titleKey);
    return titleKey;
}

bool DiscImage::readPartition(const DiscPartition &partition, uint64_t offset,
                              std::span<uint8_t> output) {
    if (!isOpen())
        return false;
    if (format == Format::RVZ) {
        auto partitions = getPartitions();
        auto found =
            std::find_if(partitions.begin(), partitions.end(),
                         [&](const DiscPartition &candidate) {
                             return candidate.offset == partition.offset;
                         });
        size_t partitionIndex = static_cast<size_t>(found - partitions.begin());
        if (found == partitions.end() || partitionIndex >= rvzPartitions.size())
            return false;
        uint64_t firstSector =
            (partition.offset + partition.dataOffset) / CLUSTER_SIZE;
        size_t copied = 0;
        try {
            while (copied < output.size()) {
                uint64_t position = offset + copied;
                uint64_t sector = firstSector + position / CLUSTER_DATA_SIZE;
                size_t inSector =
                    static_cast<size_t>(position % CLUSTER_DATA_SIZE);
                const RVZPartitionData *segment = nullptr;
                for (const auto &candidate :
                     rvzPartitions[partitionIndex].data) {
                    if (sector >= candidate.firstSector &&
                        sector <
                            candidate.firstSector + candidate.sectorCount) {
                        segment = &candidate;
                        break;
                    }
                }
                if (!segment)
                    return false;
                uint64_t sectorWithin = sector - segment->firstSector;
                uint32_t sectorsPerGroup = rvzChunkSize / CLUSTER_SIZE;
                uint32_t groupWithin =
                    static_cast<uint32_t>(sectorWithin / sectorsPerGroup);
                if (groupWithin >= segment->groupCount)
                    return false;
                uint64_t firstGroupSector =
                    static_cast<uint64_t>(groupWithin) * sectorsPerGroup;
                uint32_t sectors = static_cast<uint32_t>(std::min<uint64_t>(
                    sectorsPerGroup, segment->sectorCount - firstGroupSector));
                size_t expected =
                    static_cast<size_t>(sectors) * CLUSTER_DATA_SIZE;
                auto data = loadRVZGroup(segment->groupIndex + groupWithin,
                                         expected, true);
                size_t inGroup = static_cast<size_t>(
                    (sectorWithin - firstGroupSector) * CLUSTER_DATA_SIZE +
                    inSector);
                size_t amount =
                    std::min(output.size() - copied, data.size() - inGroup);
                std::copy_n(data.data() + inGroup, amount,
                            output.data() + copied);
                copied += amount;
            }
            return true;
        } catch (...) {
            return false;
        }
    }
    if (!encrypted)
        return offset <= partition.dataSize &&
               output.size() <= partition.dataSize - offset &&
               readRaw(partition.offset + partition.dataOffset + offset,
                       output);

    uint64_t logicalSize =
        (partition.dataSize / CLUSTER_SIZE) * CLUSTER_DATA_SIZE;
    if (offset > logicalSize || output.size() > logicalSize - offset)
        return false;

    try {
        std::array<uint8_t, 16> key = getPartitionKey(partition);
        std::array<uint8_t, CLUSTER_SIZE> cluster{};
        std::array<uint8_t, CLUSTER_DATA_SIZE> decrypted{};
        size_t copied = 0;
        while (copied < output.size()) {
            uint64_t position = offset + copied;
            uint64_t clusterIndex = position / CLUSTER_DATA_SIZE;
            size_t clusterOffset =
                static_cast<size_t>(position % CLUSTER_DATA_SIZE);
            uint64_t rawOffset = partition.offset + partition.dataOffset +
                                 clusterIndex * CLUSTER_SIZE;
            if (!readRaw(rawOffset, cluster))
                return false;

            std::span<const uint8_t, 16> iv(cluster.data() + 0x3D0, 16);
            std::span<const uint8_t> ciphertext(
                cluster.data() + CLUSTER_HEADER_SIZE, CLUSTER_DATA_SIZE);
            decryptCBC(key, iv, ciphertext, decrypted);

            size_t amount = std::min(output.size() - copied,
                                     decrypted.size() - clusterOffset);
            std::copy_n(decrypted.data() + clusterOffset, amount,
                        output.data() + copied);
            copied += amount;
        }
        return true;
    } catch (...) {
        return false;
    }
}

Executable DiscImage::getExecutable() {
    auto partition = getDataPartition();
    if (!partition)
        throw std::runtime_error("Wii disc has no data partition");

    std::array<uint8_t, 0x440> bootHeader{};
    if (!readPartition(*partition, 0, bootHeader))
        throw std::runtime_error("Failed to decrypt Wii boot header");
    uint64_t dolOffset = static_cast<uint64_t>(::readBE32(bootHeader, 0x420))
                         << 2;

    std::array<uint8_t, 0x100> dolHeader{};
    if (!readPartition(*partition, dolOffset, dolHeader))
        throw std::runtime_error("Failed to decrypt main DOL header");

    uint64_t dolSize = 0x100;
    for (uint32_t index = 0; index < 7; ++index) {
        uint64_t sectionOffset = ::readBE32(dolHeader, index * 4);
        uint64_t sectionSize = ::readBE32(dolHeader, 0x90 + index * 4);
        dolSize = std::max(dolSize, sectionOffset + sectionSize);
    }
    for (uint32_t index = 0; index < 11; ++index) {
        uint64_t sectionOffset = ::readBE32(dolHeader, 0x1C + index * 4);
        uint64_t sectionSize = ::readBE32(dolHeader, 0xAC + index * 4);
        dolSize = std::max(dolSize, sectionOffset + sectionSize);
    }
    uint64_t logicalSize =
        encrypted ? (partition->dataSize / CLUSTER_SIZE) * CLUSTER_DATA_SIZE
                  : partition->dataSize;
    if (dolSize > UINT32_MAX || dolOffset > logicalSize ||
        dolSize > logicalSize - dolOffset)
        throw std::runtime_error("Invalid main DOL range");

    std::vector<uint8_t> dol(static_cast<size_t>(dolSize));
    if (!readPartition(*partition, dolOffset, dol))
        throw std::runtime_error("Failed to decrypt main DOL");
    return Executable::parseFromDolphin(std::move(dol));
}

std::string DiscImage::log() {
    std::ostringstream output;
    std::string container = format == Format::ISO    ? "ISO"
                            : format == Format::WBFS ? "WBFS"
                                                     : "RVZ";
    output << "Wii Disc:\n"
           << "Container: " << container << '\n'
           << "Game ID: " << gameID << '\n'
           << "Title: " << title << '\n'
           << "Disc Number: " << static_cast<uint32_t>(discNumber) << '\n'
           << "Revision: " << static_cast<uint32_t>(revision) << '\n'
           << "Disc Size: " << fileSize << " bytes\n"
           << "Image Size: " << sourceSize << " bytes\n"
           << "Partition Data: "
           << (format == Format::RVZ ? "Decrypted RVZ groups"
               : encrypted           ? "Encrypted"
                                     : "Decrypted")
           << "\nPartitions:\n";
    auto partitions = getPartitions();
    for (size_t index = 0; index < partitions.size(); ++index) {
        const auto &partition = partitions[index];
        output << "  [" << index << "] " << partitionType(partition.type)
               << " offset=0x" << std::uppercase << std::hex << partition.offset
               << " data=0x" << partition.dataOffset << " size=0x"
               << partition.dataSize << std::dec << '\n';
    }
    return output.str();
}

void DiscImage::prepareBoot(const Executable &executable) {
    const auto partition = getDataPartition();
    if (!partition)
        throw std::runtime_error("Wii disc has no data partition");
    std::array<uint8_t, 0x440> header{};
    if (!readPartition(*partition, 0, header))
        throw std::runtime_error("Failed to read Wii boot header");
    const uint64_t fstOffset = static_cast<uint64_t>(::readBE32(header, 0x424)) << 2;
    const uint64_t fstSize = static_cast<uint64_t>(::readBE32(header, 0x428)) << 2;
    const uint64_t fstMaximum = static_cast<uint64_t>(::readBE32(header, 0x42C)) << 2;
    if (fstSize < 12 || fstMaximum < fstSize || fstMaximum > 0x01000000)
        throw std::runtime_error("Invalid Wii file-system table size");
    const uint32_t fstAddress = (0x817FEC60u - static_cast<uint32_t>(fstMaximum)) & ~31u;
    auto overlaps = [&](uint32_t address, uint32_t size) {
        const auto resolved = Bus::resolveAddress(address);
        return size != 0 && resolved.region == MemoryRegion::MEM1 &&
               static_cast<uint64_t>(resolved.offset) + size > (fstAddress & 0x1FFFFFFu);
    };
    for (const auto &section : executable.textSections)
        if (overlaps(section.loadAddress, section.size))
            throw std::runtime_error("Wii file-system table overlaps DOL text");
    for (const auto &section : executable.dataSections)
        if (overlaps(section.loadAddress, section.size))
            throw std::runtime_error("Wii file-system table overlaps DOL data");
    if (overlaps(executable.bssAddress, executable.bssSize))
        throw std::runtime_error("Wii file-system table overlaps DOL BSS");
    std::vector<uint8_t> fst(static_cast<size_t>(fstSize));
    if (!readPartition(*partition, fstOffset, fst))
        throw std::runtime_error("Failed to read Wii file-system table");
    Bus::writeBlock(fstAddress, fst);
    Bus::writeBlock(0, std::span<const uint8_t>(header.data(), 0x20));
    Bus::writePhysical32(0x34, fstAddress);
    Bus::writePhysical32(0x38, fstAddress);
    Bus::writePhysical32(0x3C, static_cast<uint32_t>(fstMaximum));
    Bus::writePhysical32(0x3110, fstAddress);
    Bus::writePhysical32(0x3180, ::readBE32(header, 0));
    Device::globalDevice->ios.prepareDiscBoot(partition->offset);
}
