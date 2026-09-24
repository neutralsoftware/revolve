#include "core/executable.h"
#include "core/memory.h"
#include "core/utils.h"
#include "device.h"
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <sys/types.h>

static std::string hex(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
}

static Executable parseDolphin(BigEndianStream stream) {
    Executable executable;
    executable.textSections.reserve(7);
    executable.dataSections.reserve(11);

    for (int i = 0; i < 7; i++) {
        TextSection section;
        section.startAddress = stream.readInt();
        executable.textSections.push_back(section);
    }
    for (int i = 0; i < 11; i++) {
        DataSection section;
        section.startAddress = stream.readInt();
        executable.dataSections.push_back(section);
    }

    for (int i = 0; i < 7; i++)
        executable.textSections[i].loadAddress = stream.readInt();
    for (int i = 0; i < 11; i++)
        executable.dataSections[i].loadAddress = stream.readInt();

    for (int i = 0; i < 7; i++)
        executable.textSections[i].size = stream.readInt();
    for (int i = 0; i < 11; i++)
        executable.dataSections[i].size = stream.readInt();

    executable.bssAddress = stream.readInt();
    executable.bssSize = stream.readInt();
    executable.entryPoint = stream.readInt();
    executable.data = std::make_shared<BigEndianStream>(std::move(stream));
    return executable;
}

std::string Executable::log() const {
    std::string logMessage = "Dolphin Executable:\n";

    logMessage += "Entry Point: 0x" + hex(entryPoint) + "\n";
    logMessage += "BSS Address: 0x" + hex(bssAddress) + "\n";
    logMessage += "BSS Size: 0x" + hex(bssSize) + "\n";

    logMessage += "Text Sections:\n";
    for (const auto &textSection : textSections) {
        if (textSection.size == 0)
            continue;
        logMessage += "  Start Address: 0x" + hex(textSection.startAddress) +
                      ", Load Address: 0x" + hex(textSection.loadAddress) +
                      ", Size: 0x" + hex(textSection.size) + "\n";
    }

    logMessage += "Data Sections:\n";
    for (const auto &dataSection : dataSections) {
        if (dataSection.size == 0)
            continue;
        logMessage += "  Start Address: 0x" + hex(dataSection.startAddress) +
                      ", Load Address: 0x" + hex(dataSection.loadAddress) +
                      ", Size: 0x" + hex(dataSection.size) + "\n";
    }

    return logMessage;
}

Executable Executable::parseFromDolphin(const std::string &filename) {
    return parseDolphin(BigEndianStream(filename));
}

Executable Executable::parseFromDolphin(std::vector<uint8_t> data) {
    return parseDolphin(BigEndianStream(std::move(data)));
}

Executable Executable::parseFromElf(const std::string &filename) {
    uint32_t magicCorrect = 0x7F454C46; // 0x7F 'E' 'L' 'F'
    Executable executable;

    BigEndianStream stream(filename);
    uint32_t magic = stream.readInt();
    if (magic != magicCorrect) {
        throw std::runtime_error("Invalid ELF file: incorrect magic number");
    }

    uint8_t classType = stream.readByte();
    if (classType != 1) {
        throw std::runtime_error("Invalid ELF file: unsupported class type. "
                                 "Just 32-bit is supported.");
    }

    uint8_t endianness = stream.readByte();
    if (endianness != 2) {
        throw std::runtime_error("Invalid ELF file: unsupported endianness. "
                                 "Just big-endian is supported.");
    }

    stream.skipBytes(10); // Skip version, OS ABI, ABI version, and padding

    uint16_t type = stream.readShort();
    if (type != 2) {
        throw std::runtime_error("Invalid ELF file: unsupported type. Just "
                                 "executable is supported.");
    }

    uint16_t machine = stream.readShort();
    if (machine != 0x14) {
        throw std::runtime_error("Invalid ELF file: unsupported machine. Just "
                                 "PowerPC is supported.");
    }

    stream.skipBytes(4); // Skip version

    uint32_t entryPoint = stream.readInt();
    executable.entryPoint = entryPoint;

    uint32_t programHeaderOffset = stream.readInt();
    uint32_t sectionHeaderOffset = stream.readInt();
    stream.skipBytes(4); // Skip flags
    stream.skipBytes(2); // Skip header size
    uint16_t programHeaderEntrySize = stream.readShort();
    uint16_t programHeaderCount = stream.readShort();
    uint16_t sectionHeaderEntrySize = stream.readShort();
    uint16_t sectionHeaderCount = stream.readShort();
    stream.skipBytes(2); // Skip section header string table index

    stream.moveTo(programHeaderOffset);

    for (uint16_t i = 0; i < programHeaderCount; ++i) {
        stream.moveTo(programHeaderOffset +
                      static_cast<uint32_t>(i) * programHeaderEntrySize);

        uint32_t type = stream.readInt();
        uint32_t offset = stream.readInt();
        uint32_t vaddr = stream.readInt();
        uint32_t paddr = stream.readInt();
        uint32_t filesz = stream.readInt();
        uint32_t memsz = stream.readInt();
        uint32_t flags = stream.readInt();
        uint32_t align = stream.readInt();

        if (type != 1)
            continue;

        constexpr uint32_t PF_X = 0x1;

        if (flags & PF_X) {
            TextSection section;
            section.startAddress = offset;
            section.loadAddress = vaddr;
            section.size = filesz;

            executable.textSections.push_back(section);
        } else {
            DataSection section;
            section.startAddress = offset;
            section.loadAddress = vaddr;
            section.size = filesz;

            executable.dataSections.push_back(section);
        }

        if (memsz > filesz) {
            executable.bssAddress = vaddr + filesz;
            executable.bssSize = memsz - filesz;
        }
    }
    executable.data = std::make_shared<BigEndianStream>(std::move(stream));

    return executable;
}

void Executable::loadIntoMemory() {
    GET_DEVICE();
    for (uint32_t i = 0; i < bssSize; ++i)
        Bus::writePhysical8(bssAddress + i, 0);

    for (const auto &section : textSections) {
        if (section.size == 0)
            continue;

        data->moveTo(section.startAddress);

        Bus::writeFromStream(section.loadAddress, section.size, *data);
    }

    for (const auto &section : dataSections) {
        if (section.size == 0)
            continue;

        data->moveTo(section.startAddress);

        Bus::writeFromStream(section.loadAddress, section.size, *data);
    }
}

std::optional<Executable>
Executable::parseFromFile(const std::string &filename) {
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    try {
        if (extension == "dol") {
            Executable executable = Executable::parseFromDolphin(filename);
            return executable;
        } else if (extension == "elf") {
            Executable executable = Executable::parseFromElf(filename);
            return executable;
        } else {
            std::cerr << "Error: Unsupported file extension: " << extension
                      << std::endl;
            return std::nullopt;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error parsing file: " << e.what() << std::endl;
        return std::nullopt;
    }

    return std::nullopt;
}
