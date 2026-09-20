#include "core/executable.h"
#include "core/utils.h"
#include "device.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, const char *argv[]) {
    std::vector<std::string> arguments;

    for (int i = 1; i < argc; ++i)
        arguments.emplace_back(argv[i]);

    if (arguments.empty()) {
        std::cerr << "Usage: revolve <parse|exec> <file>\n";
        return 1;
    }

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
        Logger::logObject(*exec, LogLevel::Info);
        exec->loadIntoMemory();
    } else {
        std::cerr << "Unknown command: " << arguments[0] << std::endl;
        return 1;
    }
    return 0;
}
