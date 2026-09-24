#include "SDL3/SDL.h"
#include "core/executable.h"
#include "device.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

std::tuple<bool, std::string> runCommand(const std::string &command);

namespace {
struct AdvancedTEVTest {
    std::string id;
    std::string source;
    uint32_t testCase;
    std::string title;
};

const std::vector<AdvancedTEVTest> TESTS = {
    {"ATV-LIGHT-AMBIENT", "advanced_lighting.s", 1, "Lighting ambient channel - ambient and material colors illuminate a textured quad"},
    {"ATV-LIGHT-DIFFUSE", "advanced_lighting.s", 2, "Lighting diffuse channel - a normal-driven directional light shades the surface"},
    {"ATV-LIGHT-SPOT", "advanced_lighting.s", 3, "Lighting spot attenuation - light direction and angular attenuation form a hotspot"},
    {"ATV-LIGHT-DISTANCE", "advanced_lighting.s", 4, "Lighting distance attenuation - quadratic attenuation dims distant vertices"},
    {"ATV-LIGHT-TWO", "advanced_lighting.s", 5, "Lighting two sources - red and blue lights combine across the quad"},
    {"ATV-LIGHT-SPECULAR", "advanced_lighting.s", 6, "Lighting specular channel - normal and half-angle produce a centered highlight"},
    {"ATV-LIGHT-COLOR1", "advanced_lighting.s", 7, "Lighting color channel one - TEV consumes the secondary raster channel"},
    {"ATV-NORMAL-MATRIX", "advanced_lighting.s", 8, "Normal matrix - transformed normals rotate the diffuse lighting response"},
    {"ATV-LIGHT-CLAMP", "advanced_lighting.s", 9, "Lighting accumulation clamp - multiple bright lights saturate without wrapping"},
    {"ATV-LIGHT-OFF", "advanced_lighting.s", 10, "Lighting disabled - vertex material color passes through unchanged"},
    {"ATV-TEXGEN-POS", "advanced_projection.s", 1, "Position texgen - object position generates planar texture coordinates"},
    {"ATV-TEXGEN-NORMAL", "advanced_projection.s", 2, "Normal texgen - transformed normals generate environment-map coordinates"},
    {"ATV-TEXGEN-STQ", "advanced_projection.s", 3, "STQ projection - perspective divide produces projective texture mapping"},
    {"ATV-TEXGEN-MATRIX", "advanced_projection.s", 4, "Texture matrix - scale and translation select a texture subregion"},
    {"ATV-TEXGEN-POST", "advanced_projection.s", 5, "Post texture matrix - dual transform rotates generated coordinates"},
    {"ATV-TEXGEN-NORMALIZE", "advanced_projection.s", 6, "Post texture normalization - generated vectors normalize before projection"},
    {"ATV-PROJECT-LIGHT", "advanced_projection.s", 7, "Projected light texture - position-based STQ mapping acts as a light cookie"},
    {"ATV-PROJECT-TWO", "advanced_projection.s", 8, "Two projections - base texture and projected texture use independent matrices"},
    {"ATV-TEXMTX-INDEX", "advanced_projection.s", 9, "Indexed texture matrix - per-vertex matrix indices select projection transforms"},
    {"ATV-TEXGEN-SOURCE", "advanced_projection.s", 10, "Texgen source selection - the second generator consumes a separate vertex coordinate"},
    {"ATV-TEV-LIGHTMAP", "advanced_combiner.s", 1, "Lightmap TEV - base texture multiplies a second lighting texture"},
    {"ATV-TEV-DETAIL", "advanced_combiner.s", 2, "Detail TEV - repeated detail texture modulates the base texture"},
    {"ATV-TEV-THREE", "advanced_combiner.s", 3, "Three TEV stages - base, lightmap and vertex tint combine in sequence"},
    {"ATV-TEV-KONST", "advanced_combiner.s", 4, "Konst selection - independent constant color and alpha drive a stage"},
    {"ATV-TEV-REGS", "advanced_combiner.s", 5, "TEV registers - color zero and color one preserve intermediate results"},
    {"ATV-TEV-SWAP", "advanced_combiner.s", 6, "TEV swap tables - texture and raster channels are remapped independently"},
    {"ATV-TEV-COMPARE", "advanced_combiner.s", 7, "TEV comparison operation - component comparison selects the stage result"},
    {"ATV-TEV-FOUR", "advanced_combiner.s", 8, "Four TEV stages - four ordered combiners retain previous-register flow"},
    {"ATV-INDIRECT-WARP", "advanced_combiner.s", 9, "Indirect texture warp - a second texture offsets base sampling"},
    {"ATV-INDIRECT-BUMP", "advanced_combiner.s", 10, "Indirect bump mapping - signed offsets perturb a lit texture"},
    {"ATV-INDIRECT-MTX", "advanced_combiner.s", 11, "Indirect matrix - scaled indirect coordinates distort the base texture"},
    {"ATV-INDIRECT-WRAP", "advanced_combiner.s", 12, "Indirect wrapping - wrapped offsets remain inside the configured tile"},
    {"ATV-FOG-LINEAR", "advanced_pixel.s", 1, "Linear fog - geometry fades smoothly into the fog color with depth"},
    {"ATV-FOG-PERSP", "advanced_pixel.s", 2, "Perspective fog - projected depth drives a perspective-correct fog ramp"},
    {"ATV-FOG-EXP", "advanced_pixel.s", 3, "Exponential fog - exponential density creates a nonlinear depth fade"},
    {"ATV-FOG-RANGE", "advanced_pixel.s", 4, "Fog range adjustment - horizontal range correction changes fog intensity"},
    {"ATV-Z-COMPARE", "advanced_pixel.s", 5, "Depth compare - overlapping textured quads resolve by Z order"},
    {"ATV-Z-WRITE", "advanced_pixel.s", 6, "Depth update - disabling Z writes changes later overlap visibility"},
    {"ATV-ALPHA-COMPARE", "advanced_pixel.s", 7, "Alpha compare - texture alpha cuts transparent texels before blending"},
    {"ATV-BLEND-ALPHA", "advanced_pixel.s", 8, "Source-alpha blending - translucent textured geometry blends over the background"},
    {"ATV-BLEND-SUB", "advanced_pixel.s", 9, "Subtractive blending - source color subtracts from the destination"},
    {"ATV-COLOR-MASK", "advanced_pixel.s", 10, "Color and alpha masks - independent framebuffer channels honor update masks"},
    {"ATV-SCISSOR", "advanced_pixel.s", 11, "Scissor and offset - rendering is clipped to the central EFB rectangle"},
    {"ATV-CULL", "advanced_pixel.s", 12, "Face culling - opposite winding textured triangles select the configured face"},
    {"ATV-EFB-COPY", "advanced_pixel.s", 13, "EFB copy and clear - rendered color copies to XFB before the EFB clears"},
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
    return std::string(TESTS_PATH) + "/build/advancedTEV";
}

std::string resultPath() {
    return std::string(TESTS_PATH) + "/build/advancedTEV-results.tsv";
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
    std::filesystem::create_directories(std::filesystem::path(resultPath()).parent_path());
    std::ofstream output(resultPath(), std::ios::trunc);
    if (!output)
        throw std::runtime_error("Could not save advanced TEV test results");
    for (const auto &[id, status] : results)
        output << id << '\t' << status << '\n';
}

void printTests(const std::map<std::string, std::string> &results) {
    std::cout << "\nAdvanced GX lighting, projection, fog, texture and TEV bytecode tests\n\n";
    for (size_t index = 0; index < TESTS.size(); ++index) {
        const auto &test = TESTS[index];
        auto found = results.find(test.id);
        const std::string status = found == results.end() ? "UNTESTED" : found->second;
        const char *color = status == "OK" ? "\033[32m" : status == "FAILED" ? "\033[31m" : "\033[33m";
        std::cout << std::right << std::setw(2) << index + 1 << ". " << std::left
                  << std::setw(22) << test.id << color << '[' << status
                  << "]\033[0m  " << test.title << '\n';
    }
    std::cout << "\nType a number, or 'all': " << std::flush;
}

std::string compileTest(const AdvancedTEVTest &test) {
    std::filesystem::create_directories(buildDirectory());
    const std::string prefix = buildDirectory() + "/" + test.id;
    const std::string source = std::string(TESTS_PATH) + "/gx/" + test.source;
    const std::string include = std::string(TESTS_PATH) + "/gx";
    const std::string object = prefix + ".o";
    const std::string elf = prefix + ".elf";
    const std::string dol = prefix + ".dol";
    const std::string assemble = "powerpc-eabi-as -mbroadway -I " + shellQuote(include) +
        " --defsym CASE=" + std::to_string(test.testCase) + " " + shellQuote(source) +
        " -o " + shellQuote(object);
    auto [assembled, assembleOutput] = runCommand(assemble);
    if (!assembled)
        throw std::runtime_error("Assembly failed:\n" + assembleOutput);
    const std::string link = "powerpc-eabi-ld -Ttext=0x80004000 -e _start " +
        shellQuote(object) + " -o " + shellQuote(elf);
    auto [linked, linkOutput] = runCommand(link);
    if (!linked)
        throw std::runtime_error("Linking failed:\n" + linkOutput);
    const std::string convert = "elf2dol " + shellQuote(elf) + " " + shellQuote(dol);
    auto [converted, convertOutput] = runCommand(convert);
    if (!converted)
        throw std::runtime_error("DOL conversion failed:\n" + convertOutput);
    return dol;
}

std::optional<bool> executeTest(const AdvancedTEVTest &test, Device &device) {
    Executable executable = Executable::parseFromDolphin(compileTest(test));
    executable.loadIntoMemory();
    device.cpu.reset(executable.entryPoint);
    device.cpu.setupWiiBATs();
    const std::string windowTitle = test.id + " - " + test.title;
    SDL_SetWindowTitle(device.gx.renderer->window, windowTitle.c_str());
    SDL_RaiseWindow(device.gx.renderer->window);
    std::cout << "\n" << test.id << "\n" << test.title
              << "\nTAB = OK    ENTER = FAILED\n" << std::flush;
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

int runAdvancedTEVSuite() {
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
            if (consumed != selection.size() || number == 0 || number > TESTS.size())
                throw std::out_of_range("selection");
            selected.push_back(number - 1);
        } catch (const std::exception &) {
            std::cerr << "Invalid advanced TEV test selection: " << selection << '\n';
            return 1;
        }
    }
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return 1;
    }
    size_t failed = 0;
    try {
        for (size_t index : selected) {
            const auto &test = TESTS[index];
            auto device = Device::createDevice();
            std::optional<bool> verdict = executeTest(test, *device);
            if (!verdict) {
                std::cout << "Advanced TEV test run stopped; existing results were kept.\n";
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
        std::cerr << "Advanced TEV test error: " << error.what() << '\n';
        SDL_Quit();
        return 1;
    }
    SDL_Quit();
    std::cout << "\nUpdated " << selected.size() << " advanced TEV test result"
              << (selected.size() == 1 ? "" : "s") << " in " << resultPath() << ".\n";
    return failed == 0 ? 0 : 1;
}
