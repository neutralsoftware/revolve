
#include "device.h"
#include "SDL3/SDL.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

int runBroadwaySuite();
int runDiscSuite();
int runGXSuite();
int runIOSSuite();
int runTEVSuite();
int runAdvancedTEVSuite();

std::string getTestBuildOutputPath(const std::string &fileName) {
    return std::string(TESTS_PATH) + "/build/" + fileName;
}

std::tuple<bool, std::string> runCommand(const std::string &command) {
    std::string result;

    std::string redirectedCommand = command + " 2>&1";

    FILE *pipe = popen(redirectedCommand.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed");
    }

    char *buffer = nullptr;
    size_t capacity = 0;

    while (getline(&buffer, &capacity, pipe) != -1) {
        result += buffer;
    }

    free(buffer);

    int status = pclose(pipe);

    return {status == 0, result};
}

std::string readFile(const std::string &path) {
    std::string result;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    while (file) {
        std::string line;
        std::getline(file, line);
        result += line + "\n";
    }
    return result;
}

void runExecutableSuite() {
    // Compile sources
    std::string compileCommand =
        "powerpc-eabi-as -mbroadway " + std::string(TESTS_PATH) +
        "/core/executable.s -o " + getTestBuildOutputPath("executable.o");
    std::string linkCommand = "powerpc-eabi-ld -Ttext=0x80004000 -e _start " +
                              getTestBuildOutputPath("executable.o") + " -o " +
                              getTestBuildOutputPath("executable.elf");
    std::string toDolfCommand = "elf2dol " +
                                getTestBuildOutputPath("executable.elf") + " " +
                                getTestBuildOutputPath("executable.dol");

    auto [compileSuccess, compileOutput] = runCommand(compileCommand);
    if (!compileSuccess) {
        throw std::runtime_error("Compilation failed: " + compileOutput);
    }
    auto [linkSuccess, linkOutput] = runCommand(linkCommand);
    if (!linkSuccess) {
        throw std::runtime_error("Linking failed: " + linkOutput);
    }
    auto [toDolfSuccess, toDolfOutput] = runCommand(toDolfCommand);
    if (!toDolfSuccess) {
        throw std::runtime_error("elf2dol failed: " + toDolfOutput);
    }
    auto [runDolSuccess, runDolOutput] =
        runCommand(std::string(REVOLVE_PATH) + " parse " +
                   getTestBuildOutputPath("executable.dol"));
    if (!runDolSuccess) {
        throw std::runtime_error("Dolphin execution failed: " + runDolOutput);
    }
    std::cout << runDolOutput << std::endl;
    std::cout << "-------------" << std::endl;
    auto [runElfSuccess, runElfOutput] =
        runCommand(std::string(REVOLVE_PATH) + " parse " +
                   getTestBuildOutputPath("executable.elf"));
    if (!runElfSuccess) {
        throw std::runtime_error("ELF execution failed: " + runElfOutput);
    }
    std::cout << runElfOutput << std::endl;
}

void runMemorySuite() {
    // Compile sources
    std::string compileCommand =
        "powerpc-eabi-as -mbroadway " + std::string(TESTS_PATH) +
        "/core/executable.s -o " + getTestBuildOutputPath("executable.o");
    std::string linkCommand = "powerpc-eabi-ld -Ttext=0x80004000 -e _start " +
                              getTestBuildOutputPath("executable.o") + " -o " +
                              getTestBuildOutputPath("executable.elf");
    std::string toDolfCommand = "elf2dol " +
                                getTestBuildOutputPath("executable.elf") + " " +
                                getTestBuildOutputPath("executable.dol");

    auto [compileSuccess, compileOutput] = runCommand(compileCommand);
    if (!compileSuccess) {
        throw std::runtime_error("Compilation failed: " + compileOutput);
    }
    auto [linkSuccess, linkOutput] = runCommand(linkCommand);
    if (!linkSuccess) {
        throw std::runtime_error("Linking failed: " + linkOutput);
    }
    auto [toDolfSuccess, toDolfOutput] = runCommand(toDolfCommand);
    if (!toDolfSuccess) {
        throw std::runtime_error("elf2dol failed: " + toDolfOutput);
    }
    auto [runDolSuccess, runDolOutput] =
        runCommand(std::string(REVOLVE_PATH) + " exec " +
                   getTestBuildOutputPath("executable.dol"));
    if (!runDolSuccess) {
        throw std::runtime_error("Dolphin execution failed: " + runDolOutput);
    }
    std::cout << runDolOutput << std::endl;
    std::cout << "-------------" << std::endl;
    auto [runElfSuccess, runElfOutput] =
        runCommand(std::string(REVOLVE_PATH) + " exec " +
                   getTestBuildOutputPath("executable.elf"));
    if (!runElfSuccess) {
        throw std::runtime_error("ELF execution failed: " + runElfOutput);
    }
    std::cout << runElfOutput << std::endl;
}

bool graphicsSmoke = false;

int main(int argc, char *argv[]) {
    graphicsSmoke = argc > 2 && std::string(argv[2]) == "--smoke";
    std::string suite;
    if (argc > 1) {
        suite = argv[1];
    } else {
        throw std::invalid_argument("No test suite specified. Please provide a "
                                    "test suite name as an argument.");
    }

    if (suite == "broadway")
        return runBroadwaySuite();
    if (suite == "disc")
        return runDiscSuite();
    if (suite == "gx" || suite == "graphics")
        return runGXSuite();
    if (suite == "ios")
        return runIOSSuite();
    if (suite == "tev")
        return runTEVSuite();
    if (suite == "advancedTEV")
        return runAdvancedTEVSuite();

    std::string cleanCommand = "mkdir -p " + std::string(TESTS_PATH) + "/build";
    runCommand(cleanCommand);

    if (suite == "executable") {
        runExecutableSuite();
    } else if (suite == "memory") {
        runMemorySuite();
    }

    if (suite == "clean") {
        std::string cleanCommand =
            "rm -rf " + std::string(TESTS_PATH) + "/build";
        auto [cleanSuccess, cleanOutput] = runCommand(cleanCommand);
        if (!cleanSuccess) {
            throw std::runtime_error("Clean failed: " + cleanOutput);
        }
    }
}

bool runGraphicsSmoke(Device &device, const std::string &id, bool requireColor) {
    const auto started = std::chrono::steady_clock::now();
    uint32_t steps = 0;
    while (device.pe->read(0x0E, AccessSize::U16) != 0xBEEF) {
        device.step();
        ++steps;
        if (device.cpu.state.exceptionTaken)
            throw std::runtime_error(id + ": guest CPU exception before EFB copy");
        if ((steps & 4095) == 0) {
            SDL_PumpEvents();
            if (steps >= 2000000 || std::chrono::steady_clock::now() - started >
                                        std::chrono::seconds(15))
                throw std::runtime_error(id + ": timed out before EFB copy; PC=" +
                                         utils::toHexString(device.cpu.state.cia));
        }
    }
    device.gx.renderer->presentXFB();
    constexpr uint32_t width = 640;
    constexpr uint32_t height = 528;
    std::vector<uint8_t> pixels(width * height * 4);
    uint32_t changed = 0;
    const uint8_t firstLuma = Bus::readPhysical8(0x10000);
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint32_t pair = 0x10000 + y * width * 2 + (x & ~1u) * 2;
            const int luma = Bus::readPhysical8(pair + (x & 1) * 2);
            const int u = static_cast<int>(Bus::readPhysical8(pair + 1)) - 128;
            const int v = static_cast<int>(Bus::readPhysical8(pair + 3)) - 128;
            changed += luma != firstLuma;
            const size_t output = (y * width + x) * 4;
            pixels[output] = std::clamp((298 * (luma - 16) + 409 * v + 128) >> 8, 0, 255);
            pixels[output + 1] = std::clamp((298 * (luma - 16) - 100 * u - 208 * v + 128) >> 8, 0, 255);
            pixels[output + 2] = std::clamp((298 * (luma - 16) + 516 * u + 128) >> 8, 0, 255);
            pixels[output + 3] = 255;
        }
    }
    const std::string directory = std::string(TESTS_PATH) + "/build/smoke";
    std::filesystem::create_directories(directory);
    SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32,
                                                 pixels.data(), width * 4);
    if (!surface)
        throw std::runtime_error(SDL_GetError());
    const bool saved = SDL_SaveBMP(surface, (directory + "/" + id + ".bmp").c_str());
    SDL_DestroySurface(surface);
    if (!saved)
        throw std::runtime_error(SDL_GetError());
    const double seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count();
    std::cout << "SMOKE " << id << ": " << steps << " instructions, " << seconds
              << " s, " << changed << " differing luminance pixels\n";
    if (id == "ATV-COLOR-MASK" && changed != 0)
        throw std::runtime_error(id + ": disabled color writes changed the XFB");
    if (requireColor && changed == 0)
        throw std::runtime_error(id + ": XFB contains no visible variation");
    return true;
}
