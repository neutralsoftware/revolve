#include "SDL3/SDL.h"
#include "core/executable.h"
#include "device.h"
#include "ios/ios.h"
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>

std::tuple<bool, std::string> runCommand(const std::string &command);

namespace {
constexpr uint32_t ReportAddress = 0x3000;
constexpr uint32_t ReportMagic = 0x4354524C;

class TestWiiRemoteDevice final : public IOSDevice {
  public:
    explicit TestWiiRemoteDevice(const WiiRemoteState *state) : state(state) {}

    int32_t read(uint32_t buffer, uint32_t size) override {
        if (!state || !state->connected || size < 20)
            return -1;
        uint32_t buttons = 0;
        buttons |= static_cast<uint32_t>(state->a) << 0;
        buttons |= static_cast<uint32_t>(state->b) << 1;
        buttons |= static_cast<uint32_t>(state->one) << 2;
        buttons |= static_cast<uint32_t>(state->two) << 3;
        buttons |= static_cast<uint32_t>(state->plus) << 4;
        buttons |= static_cast<uint32_t>(state->minus) << 5;
        buttons |= static_cast<uint32_t>(state->home) << 6;
        buttons |= static_cast<uint32_t>(state->dpadUp) << 7;
        buttons |= static_cast<uint32_t>(state->dpadDown) << 8;
        buttons |= static_cast<uint32_t>(state->dpadLeft) << 9;
        buttons |= static_cast<uint32_t>(state->dpadRight) << 10;
        Bus::writePhysical32(buffer, ReportMagic);
        Bus::writePhysical32(buffer + 4, buttons);
        uint32_t value = 0;
        std::memcpy(&value, &state->accelX, sizeof(value));
        Bus::writePhysical32(buffer + 8, value);
        std::memcpy(&value, &state->accelY, sizeof(value));
        Bus::writePhysical32(buffer + 12, value);
        std::memcpy(&value, &state->accelZ, sizeof(value));
        Bus::writePhysical32(buffer + 16, value);
        return 20;
    }

  private:
    const WiiRemoteState *state;
};

std::string quote(const std::string &value) {
    std::string result = "'";
    for (char character : value)
        result += character == '\'' ? "'\\''" : std::string(1, character);
    return result + "'";
}

std::string compileTest(unsigned testCase) {
    const std::string directory =
        std::string(TESTS_PATH) + "/build/controllers";
    std::filesystem::create_directories(directory);
    const std::string name = testCase == 1 ? "gamecube" : "wii";
    const std::string object = directory + "/" + name + ".o";
    const std::string elf = directory + "/" + name + ".elf";
    const std::string dol = directory + "/" + name + ".dol";
    const std::string source = std::string(TESTS_PATH) + "/controllers.s";
    auto [assembled, assembleOutput] = runCommand(
        "powerpc-eabi-as -mbroadway --defsym CASE=" + std::to_string(testCase) +
        " " + quote(source) + " -o " + quote(object));
    if (!assembled)
        throw std::runtime_error("Assembly failed:\n" + assembleOutput);
    auto [linked, linkOutput] =
        runCommand("powerpc-eabi-ld -Ttext=0x80004000 -e _start " +
                   quote(object) + " -o " + quote(elf));
    if (!linked)
        throw std::runtime_error("Linking failed:\n" + linkOutput);
    auto [converted, convertOutput] =
        runCommand("elf2dol " + quote(elf) + " " + quote(dol));
    if (!converted)
        throw std::runtime_error("DOL conversion failed:\n" + convertOutput);
    return dol;
}

void button(const char *name, bool value) {
    std::cout << std::left << std::setw(10) << name
              << (value ? "\033[30;42m PRESSED \033[0m"
                        : "\033[2m   ---   \033[0m")
              << "  ";
}

void axis(const char *name, uint8_t value) {
    constexpr int width = 25;
    const int position = value * (width - 1) / 255;
    std::string meter(width, '-');
    meter[width / 2] = '|';
    meter[position] = '#';
    std::cout << std::left << std::setw(10) << name << '[' << meter << "] "
              << std::right << std::setw(3) << static_cast<unsigned>(value)
              << '\n';
}

void printGameCube(uint32_t high, uint32_t low) {
    const uint8_t b0 = high >> 24;
    const uint8_t b1 = high >> 16;
    std::cout << "\033[2J\033[HGameCube report from guest SI bytecode\n\n";
    button("A", b0 & 1);
    button("B", b0 & 2);
    button("X", b0 & 4);
    button("Y", b0 & 8);
    button("Start", b0 & 16);
    std::cout << "\n\n";
    button("Z", b1 & 16);
    button("L", b1 & 64);
    button("R", b1 & 32);
    std::cout << "\n\n";
    button("Up", b1 & 8);
    button("Down", b1 & 4);
    button("Left", b1 & 1);
    button("Right", b1 & 2);
    std::cout << "\n\n";
    axis("Stick X", high >> 8);
    axis("Stick Y", high);
    axis("C-stick X", low >> 24);
    axis("C-stick Y", low >> 16);
    axis("Trigger L", low >> 8);
    axis("Trigger R", low);
}

float floatValue(uint32_t bits) {
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void printWii(uint32_t buttons, float x, float y, float z) {
    std::cout
        << "\033[2J\033[HWii Remote report from guest IOS IPC bytecode\n\n";
    button("A", buttons & 1);
    button("B", buttons & 2);
    button("1", buttons & 4);
    button("2", buttons & 8);
    std::cout << "\n\n";
    button("Plus", buttons & 16);
    button("Minus", buttons & 32);
    button("Home", buttons & 64);
    std::cout << "\n\n";
    button("Up", buttons & 128);
    button("Down", buttons & 256);
    button("Left", buttons & 512);
    button("Right", buttons & 1024);
    std::cout << "\n\nAccel X   " << x << "\nAccel Y   " << y << "\nAccel Z   "
              << z << '\n';
}
}

int runControllerSuite() {
    std::cout << "Native controller diagnostics\n\n"
              << "1. GameCube controller through SI\n"
              << "2. Wii Remote through IOS IPC\n\n"
              << "Select a test: " << std::flush;
    std::string selection;
    if (!std::getline(std::cin, selection) ||
        (selection != "1" && selection != "2")) {
        std::cerr << "Invalid controller test selection\n";
        return 1;
    }
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return 1;
    }
    try {
        Executable executable =
            Executable::parseFromDolphin(compileTest(selection == "1" ? 1 : 2));
        auto device = Device::createDevice();
        executable.loadIntoMemory();
        device->cpu.reset(executable.entryPoint);
        device->cpu.setupWiiBATs();
        device->ios.registerDevice("/dev/revolve/wiimote",
                                   std::make_shared<TestWiiRemoteDevice>(
                                       &device->inputManager->wiimote(0)));
        SDL_SetWindowTitle(device->gx.renderer->window,
                           selection == "1" ? "GameCube Controller Test"
                                            : "Wii Remote Test");
        SDL_RaiseWindow(device->gx.renderer->window);
        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT ||
                    (event.type == SDL_EVENT_KEY_DOWN &&
                     event.key.key == SDLK_ESCAPE))
                    running = false;
            }
            SDL_PumpEvents();
            device->inputManager->update();
            for (uint32_t step = 0; step < 4096; ++step)
                device->step();
            if (Bus::readPhysical32(ReportAddress) != ReportMagic)
                continue;
            if (selection == "1")
                printGameCube(Bus::readPhysical32(ReportAddress + 4),
                              Bus::readPhysical32(ReportAddress + 8));
            else
                printWii(Bus::readPhysical32(ReportAddress + 4),
                         floatValue(Bus::readPhysical32(ReportAddress + 8)),
                         floatValue(Bus::readPhysical32(ReportAddress + 12)),
                         floatValue(Bus::readPhysical32(ReportAddress + 16)));
            std::cout << "\nPress Escape or close the Revolve window to quit"
                      << std::flush;
        }
    } catch (const std::exception &error) {
        std::cerr << "Controller test error: " << error.what() << '\n';
        SDL_Quit();
        return 1;
    }
    SDL_Quit();
    return 0;
}
