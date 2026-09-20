#include "core/executable.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, const char *argv[]) {
    std::vector<std::string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    if (arguments[0] == "parse") {
        if (arguments.size() < 2) {
            std::cerr << "Error: No file specified for parsing." << std::endl;
            return 1;
        }
        const std::string &filename = arguments[1];
        std::string extension = filename.substr(filename.find_last_of(".") + 1);
        try {
            if (extension == "dol") {
                Executable executable = Executable::parseFromDolphin(filename);
                Logger::logObject(executable, LogLevel::Info);
            } else if (extension == "elf") {
                Executable executable = Executable::parseFromElf(filename);
                Logger::logObject(executable, LogLevel::Info);
            } else {
                std::cerr << "Error: Unsupported file extension: " << extension
                          << std::endl;
                return 1;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error parsing file: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Unknown command: " << arguments[0] << std::endl;
        return 1;
    }
    return 0;
}
