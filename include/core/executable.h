#ifndef REVOLVE_EXECUTABLE
#define REVOLVE_EXECUTABLE

#include "core/utils.h"
#include <cstdint>
#include <string>
#include <vector>

struct TextSection {
    uint32_t startAddress;
    uint32_t loadAddress;
    uint32_t size;
};

struct DataSection {
    uint32_t startAddress;
    uint32_t loadAddress;
    uint32_t size;
};

struct Executable : public Loggable {
  public:
    std::vector<TextSection> textSections;
    std::vector<DataSection> dataSections;

    uint32_t bssAddress = 0;
    uint32_t bssSize = 0;
    uint32_t entryPoint = 0;

    static Executable parseFromDolphin(const std::string &filename);
    static Executable parseFromElf(const std::string &filename);

    std::string log() const override;
    std::string getLogSystem() const override { return "Core"; }
};

#endif
