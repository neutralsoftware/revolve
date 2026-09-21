#include "disc.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>

namespace {
constexpr uint64_t CLUSTER_SIZE = 0x8000;
constexpr uint64_t CLUSTER_HEADER_SIZE = 0x400;
constexpr uint64_t CLUSTER_DATA_SIZE = 0x7C00;

const std::array<std::array<uint8_t, 16>, 2> COMMON_KEYS = {{
    {0xEB, 0xE4, 0x2A, 0x22, 0x5E, 0x85, 0x93, 0xE4, 0x48, 0xD9, 0xC5,
     0x45, 0x73, 0x81, 0xAA, 0xF7},
    {0x63, 0xB8, 0x2B, 0xB4, 0xF4, 0x61, 0x4E, 0x2E, 0x13, 0xF2, 0xFE,
     0xFB, 0xBA, 0x4C, 0x9B, 0x7E},
}};

uint32_t readBE32(std::span<const uint8_t> data, size_t offset) {
    if (offset > data.size() || data.size() - offset < 4)
        throw std::runtime_error("Invalid big-endian read");
    return (static_cast<uint32_t>(data[offset]) << 24) |
           (static_cast<uint32_t>(data[offset + 1]) << 16) |
           (static_cast<uint32_t>(data[offset + 2]) << 8) |
           static_cast<uint32_t>(data[offset + 3]);
}

void decryptCBC(std::span<const uint8_t, 16> key,
                std::span<const uint8_t, 16> iv,
                std::span<const uint8_t> input, std::span<uint8_t> output) {
    if (input.size() != output.size() || input.size() % 16 != 0)
        throw std::runtime_error("Invalid AES-CBC buffer size");

    EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
    if (!context)
        throw std::runtime_error("Failed to create AES context");

    int written = 0;
    int finalWritten = 0;
    bool success = EVP_DecryptInit_ex(context, EVP_aes_128_cbc(), nullptr,
                                      key.data(), iv.data()) == 1 &&
                   EVP_CIPHER_CTX_set_padding(context, 0) == 1 &&
                   EVP_DecryptUpdate(context, output.data(), &written,
                                     input.data(),
                                     static_cast<int>(input.size())) == 1 &&
                   EVP_DecryptFinal_ex(context, output.data() + written,
                                       &finalWritten) == 1;
    EVP_CIPHER_CTX_free(context);

    if (!success || static_cast<size_t>(written + finalWritten) != output.size())
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

    if (!file)
        return false;

    try {
        fileSize = std::filesystem::file_size(path);
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
        title.assign(reinterpret_cast<const char *>(header.data()), titleLength);
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

bool DiscImage::readRaw(uint64_t offset, std::span<uint8_t> output) {
    if (!file || offset > fileSize || output.size() > fileSize - offset)
        return false;

    file.clear();
    file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!file)
        return false;
    file.read(reinterpret_cast<char *>(output.data()),
              static_cast<std::streamsize>(output.size()));
    return static_cast<size_t>(file.gcount()) == output.size();
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
        uint64_t tableOffset =
            static_cast<uint64_t>(readBE32(descriptor + 4)) << 2;
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
            if (partition.offset > fileSize || fileSize - partition.offset < 0x2C0)
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
            size_t clusterOffset = static_cast<size_t>(position % CLUSTER_DATA_SIZE);
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
    uint64_t dolOffset = static_cast<uint64_t>(::readBE32(bootHeader, 0x420)) << 2;

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
    uint64_t logicalSize = encrypted
                               ? (partition->dataSize / CLUSTER_SIZE) *
                                     CLUSTER_DATA_SIZE
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
    output << "Wii Disc:\n"
           << "Game ID: " << gameID << '\n'
           << "Title: " << title << '\n'
           << "Disc Number: " << static_cast<uint32_t>(discNumber) << '\n'
           << "Revision: " << static_cast<uint32_t>(revision) << '\n'
           << "Size: " << fileSize << " bytes\n"
           << "Partition Data: " << (encrypted ? "Encrypted" : "Decrypted")
           << "\nPartitions:\n";
    auto partitions = getPartitions();
    for (size_t index = 0; index < partitions.size(); ++index) {
        const auto &partition = partitions[index];
        output << "  [" << index << "] " << partitionType(partition.type)
               << " offset=0x" << std::uppercase << std::hex
               << partition.offset << " data=0x" << partition.dataOffset
               << " size=0x" << partition.dataSize << std::dec << '\n';
    }
    return output.str();
}
