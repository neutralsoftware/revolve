
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

int runBroadwaySuite();
int runDiscSuite();
int runIOSSuite();

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

int main(int argc, char *argv[]) {
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
    if (suite == "ios")
        return runIOSSuite();

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
