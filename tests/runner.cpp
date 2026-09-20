
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

std::tuple<bool, std::string> runCommand(const std::string &command) {
    std::array<char, 256> buffer{};
    std::string result;

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int status = pclose(pipe);

    return std::make_tuple(status == 0, result);
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
    std::string compileCommand = "powerpc-eabi-as -mbroadway " +
                                 std::string(TESTS_PATH) +
                                 "/core/executable.s -o /tmp/executable.o";
    std::string linkCommand = "powerpc-eabi-ld -Ttext=0x80004000 -e _start "
                              "/tmp/executable.o -o /tmp/executable.elf";
    std::string toDolfCommand =
        "elf2dol /tmp/executable.elf /tmp/executable.dol";

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
        runCommand(std::string(REVOLVE_PATH) + " parse /tmp/executable.dol");
    if (!runDolSuccess) {
        throw std::runtime_error("Dolphin execution failed: " + runDolOutput);
    }
    std::cout << runDolOutput << std::endl;
    std::cout << "-------------" << std::endl;
    auto [runElfSuccess, runElfOutput] =
        runCommand(std::string(REVOLVE_PATH) + " parse /tmp/executable.elf");
    if (!runElfSuccess) {
        throw std::runtime_error("Dolphin execution failed: " + runElfOutput);
    }
    std::cout << runElfOutput << std::endl;
}

int main(int argc, char *argv[]) {
    std::string suite;
    if (argc > 1) {
        suite = argv[1];
    } else {
        throw std::invalid_argument("No test suite specified. Please provide a "
                                    "test suite name as an argument.");
    }

    if (suite == "executable") {
        runExecutableSuite();
    }
}
