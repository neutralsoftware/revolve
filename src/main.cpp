#include "core/executable.h"
#include "core/utils.h"
#include "debugger.h"
#include "device.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

int main(int argc, const char *argv[]) {
    std::vector<std::string> arguments;
    bool debug = false;

    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == "-S" || argument == "--debug")
            debug = true;
        else
            arguments.push_back(argument);
    }

    if (arguments.empty()) {
        std::cerr << "Usage: revolve [-S] <parse|exec> <file>\n"
                  << "       revolve disc <image.iso>\n"
                  << "       revolve -S <file>\n";
        return 1;
    }

    if (debug && arguments.size() == 1)
        arguments.insert(arguments.begin(), "exec");

    Device::createDevice();

    auto loadExecutable = [](const std::string &filename)
        -> std::optional<Executable> {
        size_t separator = filename.find_last_of('.');
        std::string extension =
            separator == std::string::npos ? "" : filename.substr(separator + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char character) {
                           return std::tolower(character);
                       });
        if (extension != "iso")
            return Executable::parseFromFile(filename);
        if (!Device::globalDevice->disc->open(filename)) {
            std::cerr << "Error: Failed to open Wii disc image." << std::endl;
            return std::nullopt;
        }
        try {
            return Device::globalDevice->disc->getExecutable();
        } catch (const std::exception &error) {
            std::cerr << "Error: " << error.what() << std::endl;
            return std::nullopt;
        }
    };

    if (arguments[0] == "disc") {
        if (arguments.size() < 2) {
            std::cerr << "Error: No disc image specified." << std::endl;
            return 1;
        }
        if (!Device::globalDevice->disc->open(arguments[1])) {
            std::cerr << "Error: Failed to open Wii disc image." << std::endl;
            return 1;
        }
        try {
            std::cout << Device::globalDevice->disc->log();
        } catch (const std::exception &error) {
            std::cerr << "Error reading disc: " << error.what() << std::endl;
            return 1;
        }
    } else if (arguments[0] == "parse") {
        if (arguments.size() < 2) {
            std::cerr << "Error: No file specified for parsing." << std::endl;
            return 1;
        }
        const std::string &filename = arguments[1];
        auto exec = loadExecutable(filename);
        if (exec) {
            Logger::logObject(*exec, LogLevel::Info);
        } else {
            std::cerr << "Error: Failed to parse executable." << std::endl;
            return 1;
        }

    } else if (arguments[0] == "exec") {
        if (arguments.size() < 2) {
            std::cerr << "Error: No file specified for parsing." << std::endl;
            return 1;
        }
        const std::string &filename = arguments[1];
        auto exec = loadExecutable(filename);
        if (!exec) {
            std::cerr << "Error: Failed to parse executable." << std::endl;
            return 1;
        }
        if (!debug)
            Logger::logObject(*exec, LogLevel::Info);
        exec->loadIntoMemory();

        Device::globalDevice->cpu.reset(exec->entryPoint);
        Device::globalDevice->cpu.setupWiiBATs();
        if (debug) {
            Debugger debugger(Device::globalDevice->cpu, *exec);
            debugger.run();
        } else {
            Device::globalDevice->start();
        }
    } else {
        std::cerr << "Unknown command: " << arguments[0] << std::endl;
        return 1;
    }
    return 0;
}
