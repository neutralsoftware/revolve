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
struct GXTest {
    std::string id;
    std::string source;
    uint32_t testCase;
    std::string title;
};

const std::vector<GXTest> TESTS = {
    {"GX-NOP", "control.s", 1, "GX NOP decoding - window stays at the normal clear color"},
    {"GX-CP-LOAD", "control.s", 2, "CP command loads - vertex descriptor and VAT registers accept FIFO writes"},
    {"GX-BP-LOAD", "control.s", 3, "BP command loads - pixel-engine register writes execute in order"},
    {"GX-XF-MATRIX", "control.s", 4, "XF matrix burst - twelve identity matrix words load in one command"},
    {"GX-XF-PROJECTION", "control.s", 5, "XF projection burst - orthographic projection registers load together"},
    {"GX-FIFO-LONG", "control.s", 6, "FIFO multi-block stream - commands survive more than two 32-byte blocks"},
    {"GX-BP-SEQUENCE", "control.s", 7, "BP sequential state - several raster registers retain command order"},
    {"GX-ARRAY-STATE", "control.s", 8, "CP array state - indexed base and stride registers decode"},
    {"GX-WGPIPE-BURST", "control.s", 9, "Write-gather pipe - eight word stores flush one 32-byte burst"},
    {"GX-WGPIPE-MIXED", "control.s", 10, "Write-gather pipe - byte halfword and word stores preserve big-endian order"},
    {"GX-WGPIPE-TWO-BURSTS", "control.s", 11, "Write-gather pipe - consecutive bursts advance the FIFO write pointer"},
    {"GX-FIFO-WATERMARKS", "control.s", 12, "CP FIFO watermarks - high and low thresholds update status"},
    {"GX-CP-INTERRUPTS", "control.s", 13, "CP interrupts - overflow and underflow enables plus status clearing"},
    {"GX-TRIANGLES", "primitives.s", 1, "Triangles - one RGB triangle centered in the window"},
    {"GX-TRIANGLE-STRIP", "primitives.s", 2, "Triangle strip - four corners form a two-triangle square"},
    {"GX-TRIANGLE-FAN", "primitives.s", 3, "Triangle fan - colored fan radiates from the center"},
    {"GX-QUADS", "primitives.s", 4, "Quads - four vertices form one filled colored square"},
    {"GX-LINES", "primitives.s", 5, "Lines - two independent diagonals cross at the center"},
    {"GX-LINE-STRIP", "primitives.s", 6, "Line strip - four vertices form a connected diamond path"},
    {"GX-POINTS", "primitives.s", 7, "Points - five independent points appear around the center"},
    {"GX-NOP-INTERLEAVE", "primitives.s", 8, "NOP interleave - padding commands do not disturb a magenta triangle"},
    {"GX-POS-F32", "vertex_formats.s", 1, "Direct F32 positions - full precision triangle coordinates"},
    {"GX-POS-U8", "vertex_formats.s", 2, "Direct U8 positions - unsigned fractional coordinates decode"},
    {"GX-POS-S8", "vertex_formats.s", 3, "Direct S8 positions - signed fractional coordinates decode"},
    {"GX-POS-U16", "vertex_formats.s", 4, "Direct U16 positions - unsigned 16-bit coordinates decode"},
    {"GX-POS-S16", "vertex_formats.s", 5, "Direct S16 positions - signed 16-bit coordinates decode"},
    {"GX-COLOR-RGB565", "vertex_formats.s", 6, "RGB565 colors - packed red green and blue interpolate"},
    {"GX-COLOR-RGB8", "vertex_formats.s", 7, "RGB8 colors - three-byte colors render opaque"},
    {"GX-COLOR-RGBX8", "vertex_formats.s", 8, "RGBX8 colors - ignored X byte does not affect output"},
    {"GX-COLOR-RGBA4", "vertex_formats.s", 9, "RGBA4 colors - packed four-bit channels decode"},
    {"GX-COLOR-RGBA6", "vertex_formats.s", 10, "RGBA6 colors - packed six-bit channels decode"},
    {"GX-COLOR-RGBA8", "vertex_formats.s", 11, "RGBA8 colors - full eight-bit channels interpolate"},
    {"GX-XF-IDENTITY", "transforms.s", 1, "Identity transform - baseline triangle remains centered"},
    {"GX-XF-TRANSLATE", "transforms.s", 2, "Model translation - triangle moves up and right"},
    {"GX-XF-SCALE", "transforms.s", 3, "Model scaling - triangle shrinks to half size"},
    {"GX-XF-ROTATE", "transforms.s", 4, "Model rotation - triangle rotates ninety degrees"},
    {"GX-XF-VIEWPORT", "transforms.s", 5, "Viewport registers - Wii viewport scale and origin are accepted"},
    {"GX-XF-PERSPECTIVE", "transforms.s", 6, "Perspective projection - translated depth produces a projected triangle"},
    {"GX-XF-MATRIX-INDEX", "transforms.s", 7, "Matrix index - CP selects a second transform matrix"},
    {"VI-VTIMING", "vi.s", 1, "VI vertical timing - halfword MMIO timing write while GX keeps drawing"},
    {"VI-CONTROL", "vi.s", 2, "VI display control - display-enable configuration write"},
    {"VI-HTIMING0", "vi.s", 3, "VI horizontal timing zero - paired 32-bit MMIO write"},
    {"VI-HTIMING1", "vi.s", 4, "VI horizontal timing one - burst and blanking configuration write"},
    {"VI-FB-TOP", "vi.s", 5, "VI top framebuffer - top field address register write"},
    {"VI-FB-BOTTOM", "vi.s", 6, "VI bottom framebuffer - bottom field address register write"},
    {"VI-VBEAM", "vi.s", 7, "VI vertical beam - guest bytecode reads the live scanline counter"},
    {"VI-HBEAM", "vi.s", 8, "VI horizontal beam - guest bytecode reads the live beam counter"},
    {"VI-INTERRUPT", "vi.s", 9, "VI interrupt - enabled display interrupt asserts at its target line"},
    {"VI-INTERRUPT-ACK", "vi.s", 10, "VI interrupt acknowledge - guest clears a pending display interrupt"},
    {"VI-FOUR-INTERRUPTS", "vi.s", 11, "VI four interrupts - all display interrupt slots are programmed"},
    {"VI-INTERRUPT-CLEAR", "vi.s", 12, "VI interrupt clear - all four pending sources are acknowledged"},
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
    return std::string(TESTS_PATH) + "/build/gx";
}

std::string resultPath() {
    return std::string(TESTS_PATH) + "/build/gx-results.tsv";
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
        throw std::runtime_error("Could not save GX test results");
    for (const auto &[id, status] : results)
        output << id << '\t' << status << '\n';
}

std::string statusFor(const GXTest &test,
                      const std::map<std::string, std::string> &results) {
    auto result = results.find(test.id);
    return result == results.end() ? "UNTESTED" : result->second;
}

void printTests(const std::map<std::string, std::string> &results) {
    std::cout << "\nGX, CP, XF, BP, FIFO and VI bytecode tests\n\n";
    for (size_t index = 0; index < TESTS.size(); ++index) {
        const auto &test = TESTS[index];
        const std::string status = statusFor(test, results);
        const char *color = status == "OK"       ? "\033[32m"
                            : status == "FAILED" ? "\033[31m"
                                                   : "\033[33m";
        std::cout << std::right << std::setw(2) << index + 1 << ". "
                  << std::left << std::setw(22) << test.id << color << '['
                  << status << "]\033[0m  " << test.title << '\n';
    }
    std::cout << "\nType numbers separated by commas, or 'all': " << std::flush;
}

std::string compileTest(const GXTest &test) {
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

std::optional<bool> executeTest(const GXTest &test, Device &device) {
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

int runGXSuite() {
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
            size_t start = 0;
            while (start < selection.size()) {
                const size_t end = selection.find(',', start);
                const std::string item = selection.substr(start, end - start);
                size_t consumed = 0;
                unsigned long number = std::stoul(item, &consumed);
                if (consumed != item.size() || number == 0 ||
                    number > TESTS.size())
                    throw std::out_of_range("selection");
                selected.push_back(number - 1);
                if (end == std::string::npos)
                    break;
                start = end + 1;
            }
            if (selected.empty() || selection.back() == ',')
                throw std::out_of_range("selection");
        } catch (const std::exception &) {
            std::cerr << "Invalid GX test selection: " << selection << '\n';
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
            const GXTest &test = TESTS[index];
            std::optional<bool> verdict = executeTest(test, *device);
            if (!verdict) {
                std::cout << "GX test run stopped; existing results were kept.\n";
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
        std::cerr << "GX test error: " << error.what() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Quit();
    std::cout << "\nUpdated " << selected.size() << " GX test result"
              << (selected.size() == 1 ? "" : "s") << " in " << resultPath()
              << ".\n";
    return failed == 0 ? 0 : 1;
}
