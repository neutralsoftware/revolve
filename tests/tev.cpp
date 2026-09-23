#include "SDL3/SDL.h"
#include "core/executable.h"
#include "device.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

std::tuple<bool, std::string> runCommand(const std::string &command);

namespace {
struct TEVTest {
    std::string id;
    std::string source;
    uint32_t testCase;
    std::string title;
};

const std::vector<TEVTest> TESTS = {
    {"TEX-I4", "texture_formats.s", 1, "I4 texture - tiled four-bit intensity checkerboard"},
    {"TEX-I8", "texture_formats.s", 2, "I8 texture - tiled eight-bit intensity bands"},
    {"TEX-IA4", "texture_formats.s", 3, "IA4 texture - four-bit intensity and alpha"},
    {"TEX-IA8", "texture_formats.s", 4, "IA8 texture - eight-bit intensity and alpha"},
    {"TEX-RGB565", "texture_formats.s", 5, "RGB565 texture - opaque red green blue and white tiles"},
    {"TEX-RGB5A3", "texture_formats.s", 6, "RGB5A3 texture - opaque and translucent color tiles"},
    {"TEX-RGBA8", "texture_formats.s", 7, "RGBA8 texture - split AR and GB tiled planes"},
    {"TEX-C4", "texture_formats.s", 8, "C4 texture - four-bit indices through an RGB565 palette"},
    {"TEX-C8", "texture_formats.s", 9, "C8 texture - eight-bit indices through an RGB5A3 palette"},
    {"TEX-C14X2", "texture_formats.s", 10, "C14X2 texture - fourteen-bit indices through an IA8 palette"},
    {"TEX-CMPR", "texture_formats.s", 11, "CMPR texture - four compressed color subblocks"},
    {"TEX-TRIANGLE", "texture_sampling.s", 1, "Textured triangle - direct floating-point UV coordinates"},
    {"TEX-QUAD", "texture_sampling.s", 2, "Textured quad - complete zero-to-one UV coverage"},
    {"TEX-UV-OUTSIDE", "texture_sampling.s", 3, "UV range - coordinates extend outside the texture"},
    {"TEX-WRAP-CLAMP", "texture_sampling.s", 4, "Clamp wrapping - edge texels extend beyond zero-to-one"},
    {"TEX-WRAP-REPEAT", "texture_sampling.s", 5, "Repeat wrapping - texture tiles twice across the quad"},
    {"TEX-WRAP-MIRROR", "texture_sampling.s", 6, "Mirror wrapping - alternate tiles reverse direction"},
    {"TEX-FILTER-NEAR", "texture_sampling.s", 7, "Nearest filtering - checker texels retain hard boundaries"},
    {"TEX-FILTER-LINEAR", "texture_sampling.s", 8, "Linear filtering - checker texels blend at boundaries"},
    {"TEXCOORD-S8", "texture_sampling.s", 9, "Direct S8 texture coordinates - fractional UV decoding"},
    {"TEXCOORD-S16", "texture_sampling.s", 10, "Direct S16 texture coordinates - fractional UV decoding"},
    {"TEXCOORD-INDEX8", "texture_sampling.s", 11, "Indexed texture coordinates - CP array base and stride"},
    {"TEX-MAP1", "texture_sampling.s", 12, "Texture map one - TEV selects the second texture unit"},
    {"TEV-REPLACE", "tev_stages.s", 1, "TEV replace - texture color and alpha replace raster color"},
    {"TEV-MODULATE", "tev_stages.s", 2, "TEV modulate - texture multiplies interpolated raster color"},
    {"TEV-DECAL", "tev_stages.s", 3, "TEV decal - texture alpha blends texture and raster colors"},
    {"TEV-BLEND", "tev_stages.s", 4, "TEV blend - texture color controls the blend toward white"},
    {"TEV-PASSCLR", "tev_stages.s", 5, "TEV pass color - raster color bypasses texture color"},
    {"TEV-COLOR-ADD", "tev_stages.s", 6, "TEV color add - interpolated A and B plus D"},
    {"TEV-COLOR-SUB", "tev_stages.s", 7, "TEV color subtract - subtract operation on texture and raster"},
    {"TEV-BIAS-ADDHALF", "tev_stages.s", 8, "TEV add-half bias - half is added before output"},
    {"TEV-BIAS-SUBHALF", "tev_stages.s", 9, "TEV subtract-half bias - half is removed before output"},
    {"TEV-SCALE-2", "tev_stages.s", 10, "TEV scale two - combiner result doubles"},
    {"TEV-SCALE-4", "tev_stages.s", 11, "TEV scale four - combiner result quadruples"},
    {"TEV-SCALE-HALF", "tev_stages.s", 12, "TEV divide two - combiner result halves"},
    {"TEV-CLAMP", "tev_stages.s", 13, "TEV clamp - values above the output range saturate"},
    {"TEV-ALPHA", "tev_stages.s", 14, "TEV alpha combiner - texture and raster alpha are combined"},
    {"TEV-KONST", "tev_stages.s", 15, "TEV konst - constant color and alpha feed the combiner"},
    {"TEV-REG-CHAIN", "tev_stages.s", 16, "TEV register chain - stage output passes through register zero"},
    {"TEV-TWO-STAGE", "tev_stages.s", 17, "Two TEV stages - texture replace followed by raster modulation"},
};

std::string shellQuote(const std::string &value) {
    std::string result = "'";
    for (char character : value) {
        if (character == '\'')
            result += "'\\''";
        else
            result += character;
    }
    return result + "'";
}

std::string buildDirectory() {
    return std::string(TESTS_PATH) + "/build/tev";
}

std::string resultPath() {
    return std::string(TESTS_PATH) + "/build/tev-results.tsv";
}

std::map<std::string, std::string> loadResults() {
    std::map<std::string, std::string> results;
    std::ifstream input(resultPath());
    std::string line;
    while (std::getline(input, line)) {
        size_t separator = line.find('\t');
        if (separator != std::string::npos)
            results[line.substr(0, separator)] = line.substr(separator + 1);
    }
    return results;
}

void saveResults(const std::map<std::string, std::string> &results) {
    std::filesystem::create_directories(
        std::filesystem::path(resultPath()).parent_path());
    std::ofstream output(resultPath(), std::ios::trunc);
    if (!output)
        throw std::runtime_error("Could not save TEV test results");
    for (const auto &[id, status] : results)
        output << id << '\t' << status << '\n';
}

std::string statusFor(const TEVTest &test,
                      const std::map<std::string, std::string> &results) {
    auto result = results.find(test.id);
    return result == results.end() ? "UNTESTED" : result->second;
}

void printTests(const std::map<std::string, std::string> &results) {
    std::cout << "\nGX texture, sampling and TEV bytecode tests\n\n";
    for (size_t index = 0; index < TESTS.size(); ++index) {
        const auto &test = TESTS[index];
        const std::string status = statusFor(test, results);
        const char *color = status == "OK"       ? "\033[32m"
                            : status == "FAILED" ? "\033[31m"
                                                   : "\033[33m";
        std::cout << std::right << std::setw(2) << index + 1 << ". "
                  << std::left << std::setw(20) << test.id << color << '['
                  << status << "]\033[0m  " << test.title << '\n';
    }
    std::cout << "\nType a number, or 'all': " << std::flush;
}

std::string compileTest(const TEVTest &test) {
    std::filesystem::create_directories(buildDirectory());
    const std::string prefix = buildDirectory() + "/" + test.id;
    const std::string source = std::string(TESTS_PATH) + "/gx/" + test.source;
    const std::string include = std::string(TESTS_PATH) + "/gx";
    const std::string object = prefix + ".o";
    const std::string elf = prefix + ".elf";
    const std::string dol = prefix + ".dol";

    const std::string assemble =
        "powerpc-eabi-as -mbroadway -I " + shellQuote(include) +
        " --defsym CASE=" + std::to_string(test.testCase) + " " +
        shellQuote(source) + " -o " + shellQuote(object);
    auto [assembled, assembleOutput] = runCommand(assemble);
    if (!assembled)
        throw std::runtime_error("Assembly failed:\n" + assembleOutput);

    const std::string link = "powerpc-eabi-ld -Ttext=0x80004000 -e _start " +
                             shellQuote(object) + " -o " + shellQuote(elf);
    auto [linked, linkOutput] = runCommand(link);
    if (!linked)
        throw std::runtime_error("Linking failed:\n" + linkOutput);

    const std::string convert =
        "elf2dol " + shellQuote(elf) + " " + shellQuote(dol);
    auto [converted, convertOutput] = runCommand(convert);
    if (!converted)
        throw std::runtime_error("DOL conversion failed:\n" + convertOutput);

    return dol;
}

std::optional<bool> executeTest(const TEVTest &test, Device &device) {
    const std::string dol = compileTest(test);
    Executable executable = Executable::parseFromDolphin(dol);
    executable.loadIntoMemory();
    device.cpu.reset(executable.entryPoint);
    device.cpu.setupWiiBATs();

    const std::string windowTitle = test.id + " - " + test.title;
    SDL_SetWindowTitle(device.gx.renderer->window, windowTitle.c_str());
    SDL_RaiseWindow(device.gx.renderer->window);

    std::cout << "\n" << test.id << "\n" << test.title
              << "\nTAB = OK    ENTER = FAILED\n"
              << std::flush;

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                return std::nullopt;
            if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
                continue;
            if (event.key.key == SDLK_TAB)
                return true;
            if (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)
                return false;
        }
        for (uint32_t step = 0; step < 4096; ++step)
            device.step();
    }
}
}

int runTEVSuite() {
    auto results = loadResults();
    printTests(results);

    std::string selection;
    if (!std::getline(std::cin, selection))
        return 1;

    std::vector<size_t> selected;
    if (selection == "all") {
        for (size_t index = 0; index < TESTS.size(); ++index)
            selected.push_back(index);
    } else {
        try {
            size_t consumed = 0;
            unsigned long number = std::stoul(selection, &consumed);
            if (consumed != selection.size() || number == 0 ||
                number > TESTS.size())
                throw std::out_of_range("selection");
            selected.push_back(number - 1);
        } catch (const std::exception &) {
            std::cerr << "Invalid TEV test selection: " << selection << '\n';
            return 1;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return 1;
    }

    auto device = Device::createDevice();
    size_t failed = 0;
    try {
        for (size_t index : selected) {
            const TEVTest &test = TESTS[index];
            std::optional<bool> verdict = executeTest(test, *device);
            if (!verdict) {
                std::cout << "TEV test run stopped; existing results were kept.\n";
                SDL_Quit();
                return 1;
            }
            results[test.id] = *verdict ? "OK" : "FAILED";
            saveResults(results);
            if (*verdict)
                std::cout << "\033[32m[OK]\033[0m " << test.id << '\n';
            else {
                ++failed;
                std::cout << "\033[31m[FAILED]\033[0m " << test.id << '\n';
            }
        }
    } catch (const std::exception &error) {
        std::cerr << "TEV test error: " << error.what() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Quit();
    std::cout << "\nUpdated " << selected.size() << " TEV test result"
              << (selected.size() == 1 ? "" : "s") << " in " << resultPath()
              << ".\n";
    return failed == 0 ? 0 : 1;
}
