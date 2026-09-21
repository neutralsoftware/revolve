#include "core/executable.h"
#include "core/utils.h"
#include "debugger.h"
#include "device.h"
#include <iostream>
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
                  << "       revolve -S <file>\n";
        return 1;
    }

    if (debug && arguments.size() == 1)
        arguments.insert(arguments.begin(), "exec");

    Device::createDevice();

    if (arguments[0] == "parse") {
        if (arguments.size() < 2) {
            std::cerr << "Error: No file specified for parsing." << std::endl;
            return 1;
        }
        const std::string &filename = arguments[1];
        auto exec = Executable::parseFromFile(filename);
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
        auto exec = Executable::parseFromFile(filename);
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
            Device::globalDevice->cpu.start();
        }
    } else {
        std::cerr << "Unknown command: " << arguments[0] << std::endl;
        return 1;
    }
    return 0;
}
