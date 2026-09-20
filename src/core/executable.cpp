#include "core/executable.h"
#include "core/utils.h"
#include <sstream>
#include <string>

static std::string hex(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
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
    BigEndianStream stream(filename);

    Executable executable;
    executable.textSections.reserve(7);
    executable.dataSections.reserve(11);

    // Start addresses
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

    // Loading Addresses
    for (int i = 0; i < 7; i++) {
        executable.textSections[i].loadAddress = stream.readInt();
    }
    for (int i = 0; i < 11; i++) {
        executable.dataSections[i].loadAddress = stream.readInt();
    }

    // Sizes
    for (int i = 0; i < 7; i++) {
        executable.textSections[i].size = stream.readInt();
    }
    for (int i = 0; i < 11; i++) {
        executable.dataSections[i].size = stream.readInt();
    }
    executable.bssAddress = stream.readInt();
    executable.bssSize = stream.readInt();
    executable.entryPoint = stream.readInt();

    return executable;
}
