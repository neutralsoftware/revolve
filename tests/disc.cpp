#include "disc.h"
#include "core/utils.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <openssl/evp.h>
#include <span>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

namespace {
constexpr uint64_t PARTITION_OFFSET = 0x50000;
constexpr uint64_t DATA_OFFSET = 0x10000;
constexpr uint64_t CLUSTER_SIZE = 0x8000;
constexpr uint64_t CLUSTER_DATA_SIZE = 0x7C00;

const std::array<uint8_t, 16> COMMON_KEY = {0xEB, 0xE4, 0x2A, 0x22, 0x5E, 0x85,
                                            0x93, 0xE4, 0x48, 0xD9, 0xC5, 0x45,
                                            0x73, 0x81, 0xAA, 0xF7};

const std::array<uint8_t, 16> TITLE_KEY = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                                           0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
                                           0x0C, 0x0D, 0x0E, 0x0F};

void expect(uint64_t actual, uint64_t expected, const std::string &field) {
    if (actual != expected)
        throw std::runtime_error(field + ": expected " +
                                 utils::toHexString(expected) + ", got " +
                                 utils::toHexString(actual));
}

void expect(const std::string &actual, const std::string &expected,
            const std::string &field) {
    if (actual != expected)
        throw std::runtime_error(field + ": expected " + expected + ", got " +
                                 actual);
}

void writeBE32(std::span<uint8_t> data, size_t offset, uint32_t value) {
    data[offset] = static_cast<uint8_t>(value >> 24);
    data[offset + 1] = static_cast<uint8_t>(value >> 16);
    data[offset + 2] = static_cast<uint8_t>(value >> 8);
    data[offset + 3] = static_cast<uint8_t>(value);
}

void writeBE64(std::span<uint8_t> data, size_t offset, uint64_t value) {
    writeBE32(data, offset, static_cast<uint32_t>(value >> 32));
    writeBE32(data, offset + 4, static_cast<uint32_t>(value));
}

void encryptCBC(std::span<const uint8_t, 16> key,
                std::span<const uint8_t, 16> iv, std::span<const uint8_t> input,
                std::span<uint8_t> output) {
    EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
    if (!context)
        throw std::runtime_error("Failed to create AES test context");
    int written = 0;
    int finalWritten = 0;
    bool success =
        EVP_EncryptInit_ex(context, EVP_aes_128_cbc(), nullptr, key.data(),
                           iv.data()) == 1 &&
        EVP_CIPHER_CTX_set_padding(context, 0) == 1 &&
        EVP_EncryptUpdate(context, output.data(), &written, input.data(),
                          static_cast<int>(input.size())) == 1 &&
        EVP_EncryptFinal_ex(context, output.data() + written, &finalWritten) ==
            1;
    EVP_CIPHER_CTX_free(context);
    if (!success ||
        static_cast<size_t>(written + finalWritten) != output.size())
        throw std::runtime_error("AES test encryption failed");
}

std::string createDiscImage() {
    std::vector<uint8_t> image(PARTITION_OFFSET + DATA_OFFSET + CLUSTER_SIZE);
    const std::string gameID = "RTESTE";
    std::copy(gameID.begin(), gameID.end(), image.begin());
    image[6] = 1;
    image[7] = 2;
    writeBE32(image, 0x18, 0x5D1C9EA3);
    const std::string title = "Revolve Encrypted Disc Test";
    std::copy(title.begin(), title.end(), image.begin() + 0x20);

    writeBE32(image, 0x40000, 1);
    writeBE32(image, 0x40004, 0x40020 >> 2);
    writeBE32(image, 0x40020, PARTITION_OFFSET >> 2);
    writeBE32(image, 0x40024, 0);
    writeBE32(image, PARTITION_OFFSET + 0x2B8, DATA_OFFSET >> 2);
    writeBE32(image, PARTITION_OFFSET + 0x2BC, CLUSTER_SIZE >> 2);

    const std::array<uint8_t, 8> titleID = {0x00, 0x01, 0x00, 0x00,
                                            0x52, 0x54, 0x53, 0x54};
    std::copy(titleID.begin(), titleID.end(),
              image.begin() + PARTITION_OFFSET + 0x1DC);
    std::array<uint8_t, 16> titleIV{};
    std::copy(titleID.begin(), titleID.end(), titleIV.begin());
    std::array<uint8_t, 16> encryptedTitleKey{};
    encryptCBC(COMMON_KEY, titleIV, TITLE_KEY, encryptedTitleKey);
    std::copy(encryptedTitleKey.begin(), encryptedTitleKey.end(),
              image.begin() + PARTITION_OFFSET + 0x1BF);

    std::vector<uint8_t> plaintext(CLUSTER_DATA_SIZE);
    writeBE32(plaintext, 0x420, 0x1000 >> 2);
    writeBE32(plaintext, 0x1000, 0x100);
    writeBE32(plaintext, 0x1048, 0x80004000);
    writeBE32(plaintext, 0x1090, 4);
    writeBE32(plaintext, 0x10D8, 0x80005000);
    writeBE32(plaintext, 0x10DC, 0x20);
    writeBE32(plaintext, 0x10E0, 0x80004000);
    writeBE32(plaintext, 0x1100, 0x60000000);

    std::array<uint8_t, 16> clusterIV = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                                         0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
                                         0x1C, 0x1D, 0x1E, 0x1F};
    uint64_t clusterOffset = PARTITION_OFFSET + DATA_OFFSET;
    std::copy(clusterIV.begin(), clusterIV.end(),
              image.begin() + clusterOffset + 0x3D0);
    std::span<uint8_t> ciphertext(image.data() + clusterOffset + 0x400,
                                  CLUSTER_DATA_SIZE);
    encryptCBC(TITLE_KEY, clusterIV, plaintext, ciphertext);

    std::string path = (std::filesystem::temp_directory_path() /
                        ("revolve-disc-" + std::to_string(getpid()) + ".iso"))
                           .string();
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char *>(image.data()),
                 static_cast<std::streamsize>(image.size()));
    if (!output)
        throw std::runtime_error("Failed to write test disc image");
    return path;
}

std::vector<uint8_t> readFile(const std::string &path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input)
        throw std::runtime_error("Failed to read test image");
    size_t size = static_cast<size_t>(input.tellg());
    input.seekg(0);
    std::vector<uint8_t> data(size);
    input.read(reinterpret_cast<char *>(data.data()),
               static_cast<std::streamsize>(data.size()));
    if (!input)
        throw std::runtime_error("Failed to read test image data");
    return data;
}

std::string createWBFSImage(const std::string &isoPath) {
    auto iso = readFile(isoPath);
    constexpr size_t sectorSize = 0x200000;
    std::vector<uint8_t> wbfs(sectorSize * 2);
    std::copy_n("WBFS", 4, wbfs.begin());
    writeBE32(wbfs, 4, 2);
    wbfs[8] = 9;
    wbfs[9] = 21;
    wbfs[12] = 1;
    std::copy_n(iso.begin(), 0x100, wbfs.begin() + 0x200);
    wbfs[0x300] = 0;
    wbfs[0x301] = 1;
    std::copy(iso.begin(), iso.end(), wbfs.begin() + sectorSize);
    std::string path = (std::filesystem::temp_directory_path() /
                        ("revolve-disc-" + std::to_string(getpid()) + ".wbfs"))
                           .string();
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char *>(wbfs.data()),
                 static_cast<std::streamsize>(wbfs.size()));
    return path;
}

std::string createRVZImage(const std::string &isoPath) {
    auto iso = readFile(isoPath);
    DiscImage source;
    if (!source.open(isoPath))
        throw std::runtime_error("Failed to open source ISO");
    auto partition = source.getDataPartition();
    if (!partition)
        throw std::runtime_error("Missing source data partition");
    std::vector<uint8_t> partitionData(CLUSTER_DATA_SIZE);
    if (!source.readPartition(*partition, 0, partitionData))
        throw std::runtime_error("Failed to decrypt source partition");

    constexpr size_t partOffset = 0x124;
    constexpr size_t rawOffset = partOffset + 48;
    constexpr size_t groupOffset = rawOffset + 24;
    constexpr size_t dataOffset = groupOffset + 24;
    size_t partitionGroupOffset = dataOffset + iso.size();
    std::vector<uint8_t> rvz(partitionGroupOffset + 4 + partitionData.size());
    rvz[0] = 'R';
    rvz[1] = 'V';
    rvz[2] = 'Z';
    rvz[3] = 1;
    writeBE32(rvz, 4, 0x01000000);
    writeBE32(rvz, 8, 0x01000000);
    writeBE32(rvz, 0xC, 0xDC);
    writeBE64(rvz, 0x24, iso.size());
    writeBE64(rvz, 0x2C, rvz.size());

    size_t disc = 0x48;
    writeBE32(rvz, disc, 2);
    writeBE32(rvz, disc + 4, 0);
    writeBE32(rvz, disc + 0xC, 0x80000);
    std::copy_n(iso.begin(), 0x80, rvz.begin() + disc + 0x10);
    writeBE32(rvz, disc + 0x90, 1);
    writeBE32(rvz, disc + 0x94, 48);
    writeBE64(rvz, disc + 0x98, partOffset);
    writeBE32(rvz, disc + 0xB4, 1);
    writeBE64(rvz, disc + 0xB8, rawOffset);
    writeBE32(rvz, disc + 0xC0, 24);
    writeBE32(rvz, disc + 0xC4, 2);
    writeBE64(rvz, disc + 0xC8, groupOffset);
    writeBE32(rvz, disc + 0xD0, 24);

    size_t part = partOffset + 16;
    writeBE32(
        rvz, part,
        static_cast<uint32_t>((PARTITION_OFFSET + DATA_OFFSET) / CLUSTER_SIZE));
    writeBE32(rvz, part + 4, 1);
    writeBE32(rvz, part + 8, 1);
    writeBE32(rvz, part + 12, 1);

    writeBE64(rvz, rawOffset, 0);
    writeBE64(rvz, rawOffset + 8, iso.size());
    writeBE32(rvz, rawOffset + 16, 0);
    writeBE32(rvz, rawOffset + 20, 1);

    writeBE32(rvz, groupOffset, dataOffset >> 2);
    writeBE32(rvz, groupOffset + 4, iso.size());
    writeBE32(rvz, groupOffset + 8, 0);
    writeBE32(rvz, groupOffset + 12, partitionGroupOffset >> 2);
    writeBE32(rvz, groupOffset + 16, partitionData.size() + 4);
    writeBE32(rvz, groupOffset + 20, 0);

    std::copy(iso.begin(), iso.end(), rvz.begin() + dataOffset);
    std::copy(partitionData.begin(), partitionData.end(),
              rvz.begin() + partitionGroupOffset + 4);
    std::string path = (std::filesystem::temp_directory_path() /
                        ("revolve-disc-" + std::to_string(getpid()) + ".rvz"))
                           .string();
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char *>(rvz.data()),
                 static_cast<std::streamsize>(rvz.size()));
    return path;
}

struct DiscTest {
    const char *name;
    std::string path;
    std::function<void(DiscImage &)> run;
};
}

int runDiscSuite() {
    std::string path = createDiscImage();
    std::string wbfsPath = createWBFSImage(path);
    std::string rvzPath = createRVZImage(path);
    std::vector<DiscTest> tests;

    tests.push_back(
        {"DISC_INFO", path, [](DiscImage &disc) {
             expect(disc.getGameID(), "RTESTE", "game ID");
             expect(disc.getTitle(), "Revolve Encrypted Disc Test", "title");
             expect(disc.getDiscNumber(), 1, "disc number");
             expect(disc.getRevision(), 2, "revision");
             auto partitions = disc.getPartitions();
             expect(partitions.size(), 1, "partition count");
             expect(partitions[0].offset, PARTITION_OFFSET, "partition offset");
             expect(partitions[0].dataOffset, DATA_OFFSET,
                    "partition data offset");
         }});

    tests.push_back(
        {"ENCRYPTED_READ", path, [](DiscImage &disc) {
             auto partition = disc.getDataPartition();
             if (!partition)
                 throw std::runtime_error("Missing data partition");
             std::array<uint8_t, 4> instruction{};
             if (!disc.readPartition(*partition, 0x1100, instruction))
                 throw std::runtime_error("Encrypted read failed");
             expect(instruction[0], 0x60, "instruction byte 0");
             expect(instruction[1], 0, "instruction byte 1");
             expect(instruction[2], 0, "instruction byte 2");
             expect(instruction[3], 0, "instruction byte 3");
         }});

    tests.push_back(
        {"DISC_DOL", path, [](DiscImage &disc) {
             Executable executable = disc.getExecutable();
             expect(executable.entryPoint, 0x80004000, "DOL entry point");
             expect(executable.textSections[0].startAddress, 0x100,
                    "text file offset");
             expect(executable.textSections[0].loadAddress, 0x80004000,
                    "text load address");
             expect(executable.textSections[0].size, 4, "text section size");
             expect(executable.bssAddress, 0x80005000, "BSS address");
             expect(executable.bssSize, 0x20, "BSS size");
         }});

    tests.push_back({"WBFS_DOL", wbfsPath, [](DiscImage &disc) {
                         Executable executable = disc.getExecutable();
                         expect(executable.entryPoint, 0x80004000,
                                "WBFS DOL entry point");
                     }});

    tests.push_back({"RVZ_DOL", rvzPath, [](DiscImage &disc) {
                         Executable executable = disc.getExecutable();
                         expect(executable.entryPoint, 0x80004000,
                                "RVZ DOL entry point");
                     }});

    size_t passed = 0;
    for (const auto &test : tests) {
        DiscImage disc;
        std::cout << std::left << std::setw(20) << test.name << " ..... ";
        try {
            if (!disc.open(test.path))
                throw std::runtime_error("Failed to open test disc image");
            test.run(disc);
            ++passed;
            std::cout << "\033[32m[PASSED]\033[0m\n";
        } catch (const std::exception &error) {
            std::cout << "\033[31m[FAILED]\033[0m " << error.what() << '\n';
        }
    }

    std::filesystem::remove(path);
    std::filesystem::remove(wbfsPath);
    std::filesystem::remove(rvzPath);
    std::cout << '\n'
              << passed << "/" << tests.size() << " Disc tests passed.\n";
    return passed == tests.size() ? 0 : 1;
}
