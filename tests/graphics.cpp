#include "device.h"
#include "graphics/video_interface.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int WIDTH = 640;
constexpr int HEIGHT = 480;

struct GraphicsTest {
    std::string name;
    std::function<bool()> validate;
    std::function<void(SDL_Renderer *)> render;
};

void setColor(SDL_Renderer *renderer, uint8_t red, uint8_t green, uint8_t blue,
              uint8_t alpha = 255) {
    SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
}

void fill(SDL_Renderer *renderer, float x, float y, float width, float height) {
    SDL_FRect rectangle{x, y, width, height};
    SDL_RenderFillRect(renderer, &rectangle);
}

void clear(SDL_Renderer *renderer, uint8_t red, uint8_t green, uint8_t blue) {
    setColor(renderer, red, green, blue);
    SDL_RenderClear(renderer);
}

void label(SDL_Renderer *renderer, const std::string &name) {
    setColor(renderer, 0, 0, 0, 210);
    fill(renderer, 0, 0, WIDTH, 38);
    setColor(renderer, 255, 255, 255);
    SDL_RenderDebugText(renderer, 10, 7, name.c_str());
    SDL_RenderDebugText(renderer, 10, 21, "TAB = OK    ENTER = BAD");
}

void drawCheckerboard(SDL_Renderer *renderer, int tileSize) {
    clear(renderer, 0, 0, 0);
    for (int y = 0; y < HEIGHT; y += tileSize) {
        for (int x = 0; x < WIDTH; x += tileSize) {
            uint8_t value = ((x / tileSize) + (y / tileSize)) % 2 ? 255 : 0;
            setColor(renderer, value, value, value);
            fill(renderer, static_cast<float>(x), static_cast<float>(y),
                 static_cast<float>(tileSize), static_cast<float>(tileSize));
        }
    }
}

void drawGrid(SDL_Renderer *renderer, int spacing) {
    clear(renderer, 20, 20, 24);
    setColor(renderer, 70, 70, 80);
    for (int x = 0; x < WIDTH; x += spacing)
        SDL_RenderLine(renderer, static_cast<float>(x), 0,
                       static_cast<float>(x), HEIGHT - 1.0f);
    for (int y = 0; y < HEIGHT; y += spacing)
        SDL_RenderLine(renderer, 0, static_cast<float>(y), WIDTH - 1.0f,
                       static_cast<float>(y));
    setColor(renderer, 255, 64, 64);
    SDL_RenderLine(renderer, WIDTH / 2.0f, 0, WIDTH / 2.0f, HEIGHT - 1.0f);
    setColor(renderer, 64, 255, 128);
    SDL_RenderLine(renderer, 0, HEIGHT / 2.0f, WIDTH - 1.0f, HEIGHT / 2.0f);
}

void drawCircle(SDL_Renderer *renderer, float centerX, float centerY,
                float radius) {
    constexpr int SEGMENTS = 360;
    std::vector<SDL_FPoint> points;
    points.reserve(SEGMENTS + 1);
    for (int index = 0; index <= SEGMENTS; ++index) {
        float angle = static_cast<float>(index) * 2.0f *
                      static_cast<float>(std::acos(-1.0)) / SEGMENTS;
        points.push_back({centerX + std::cos(angle) * radius,
                          centerY + std::sin(angle) * radius});
    }
    SDL_RenderLines(renderer, points.data(), static_cast<int>(points.size()));
}

bool waitForVerdict() {
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            throw std::runtime_error("Graphics test window closed");
        if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat)
            continue;
        if (event.key.key == SDLK_TAB)
            return true;
        if (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)
            return false;
    }
    throw std::runtime_error(SDL_GetError());
}

void addHostTests(std::vector<GraphicsTest> &tests) {
    auto alwaysValid = []() { return true; };
    tests.push_back({"HOST_CLEAR_BLACK", alwaysValid,
                     [](SDL_Renderer *renderer) { clear(renderer, 0, 0, 0); }});
    tests.push_back({"HOST_SOLID_RED", alwaysValid, [](SDL_Renderer *renderer) {
                         clear(renderer, 255, 0, 0);
                     }});
    tests.push_back(
        {"HOST_SOLID_GREEN", alwaysValid,
         [](SDL_Renderer *renderer) { clear(renderer, 0, 255, 0); }});
    tests.push_back(
        {"HOST_SOLID_BLUE", alwaysValid,
         [](SDL_Renderer *renderer) { clear(renderer, 0, 0, 255); }});
    tests.push_back(
        {"HOST_SMPTE_COLOR_BARS", alwaysValid, [](SDL_Renderer *renderer) {
             const uint8_t colors[8][3] = {
                 {255, 255, 255}, {255, 255, 0}, {0, 255, 255}, {0, 255, 0},
                 {255, 0, 255},   {255, 0, 0},   {0, 0, 255},   {0, 0, 0}};
             for (int index = 0; index < 8; ++index) {
                 setColor(renderer, colors[index][0], colors[index][1],
                          colors[index][2]);
                 fill(renderer, index * 80.0f, 0, 80, HEIGHT);
             }
         }});
    tests.push_back(
        {"HOST_GRAYSCALE_RAMP", alwaysValid, [](SDL_Renderer *renderer) {
             for (int x = 0; x < WIDTH; ++x) {
                 uint8_t value = static_cast<uint8_t>(x * 255 / (WIDTH - 1));
                 setColor(renderer, value, value, value);
                 SDL_RenderLine(renderer, static_cast<float>(x), 0,
                                static_cast<float>(x), HEIGHT);
             }
         }});
    tests.push_back({"HOST_RGB_HORIZONTAL_GRADIENT", alwaysValid,
                     [](SDL_Renderer *renderer) {
                         for (int x = 0; x < WIDTH; ++x) {
                             uint8_t red =
                                 static_cast<uint8_t>(x * 255 / (WIDTH - 1));
                             uint8_t blue = static_cast<uint8_t>(255 - red);
                             setColor(renderer, red, 128, blue);
                             SDL_RenderLine(renderer, static_cast<float>(x), 0,
                                            static_cast<float>(x), HEIGHT);
                         }
                     }});
    tests.push_back(
        {"HOST_PIXEL_CHECKERBOARD", alwaysValid,
         [](SDL_Renderer *renderer) { drawCheckerboard(renderer, 1); }});
    tests.push_back(
        {"HOST_TILE_CHECKERBOARD", alwaysValid,
         [](SDL_Renderer *renderer) { drawCheckerboard(renderer, 32); }});
    tests.push_back(
        {"HOST_LINE_RASTERIZATION", alwaysValid, [](SDL_Renderer *renderer) {
             clear(renderer, 0, 0, 0);
             for (int x = 0; x < WIDTH; x += 20) {
                 setColor(renderer, static_cast<uint8_t>(x * 255 / WIDTH), 255,
                          180);
                 SDL_RenderLine(renderer, WIDTH / 2.0f, HEIGHT / 2.0f,
                                static_cast<float>(x), 0);
                 SDL_RenderLine(renderer, WIDTH / 2.0f, HEIGHT / 2.0f,
                                static_cast<float>(x), HEIGHT - 1);
             }
         }});
    tests.push_back({"HOST_RECTANGLE_ALIGNMENT_GRID", alwaysValid,
                     [](SDL_Renderer *renderer) { drawGrid(renderer, 16); }});
    tests.push_back(
        {"HOST_ALPHA_BLENDING", alwaysValid, [](SDL_Renderer *renderer) {
             clear(renderer, 32, 32, 32);
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
             setColor(renderer, 255, 0, 0, 160);
             fill(renderer, 100, 100, 260, 220);
             setColor(renderer, 0, 128, 255, 160);
             fill(renderer, 280, 160, 260, 220);
             SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
         }});
    tests.push_back(
        {"HOST_OVERSCAN_SAFE_AREA", alwaysValid, [](SDL_Renderer *renderer) {
             clear(renderer, 20, 20, 20);
             setColor(renderer, 255, 255, 255);
             SDL_FRect outer{0.5f, 0.5f, WIDTH - 1.0f, HEIGHT - 1.0f};
             SDL_RenderRect(renderer, &outer);
             setColor(renderer, 255, 220, 0);
             SDL_FRect safe{32.5f, 24.5f, WIDTH - 65.0f, HEIGHT - 49.0f};
             SDL_RenderRect(renderer, &safe);
         }});
    tests.push_back(
        {"HOST_ASPECT_RATIO_CIRCLES", alwaysValid, [](SDL_Renderer *renderer) {
             drawGrid(renderer, 40);
             setColor(renderer, 255, 255, 255);
             drawCircle(renderer, WIDTH / 2.0f, HEIGHT / 2.0f, 180);
             drawCircle(renderer, WIDTH / 2.0f, HEIGHT / 2.0f, 90);
         }});
    tests.push_back(
        {"HOST_SCISSOR_CLIPPING", alwaysValid, [](SDL_Renderer *renderer) {
             clear(renderer, 30, 30, 30);
             SDL_Rect clip{160, 120, 320, 240};
             SDL_SetRenderClipRect(renderer, &clip);
             for (int y = 0; y < HEIGHT; y += 20) {
                 setColor(renderer, 255, static_cast<uint8_t>(y * 255 / HEIGHT),
                          64);
                 fill(renderer, 0, static_cast<float>(y), WIDTH, 10);
             }
             SDL_SetRenderClipRect(renderer, nullptr);
             setColor(renderer, 255, 255, 255);
             SDL_FRect border{160, 120, 320, 240};
             SDL_RenderRect(renderer, &border);
         }});
}

void addVITests(std::vector<GraphicsTest> &tests, Device &device) {
    tests.push_back({"VI_PHYSICAL_MMIO_16_BIT",
                     []() {
                         Bus::write16(VI_BASE, 0x1357);
                         return Bus::read16(VI_BASE) == 0x1357;
                     },
                     [](SDL_Renderer *renderer) {
                         for (int x = 0; x < WIDTH; x += 40) {
                             uint8_t value = (x / 40) % 2 ? 255 : 0;
                             setColor(renderer, value, 80, 255 - value);
                             fill(renderer, static_cast<float>(x), 0, 40,
                                  HEIGHT);
                         }
                     }});
    tests.push_back({"VI_PHYSICAL_MMIO_32_BIT",
                     []() {
                         Bus::write32(VI_BASE + 0x04, 0x1234ABCD);
                         return Bus::read32(VI_BASE + 0x04) == 0x1234ABCD;
                     },
                     [](SDL_Renderer *renderer) { drawGrid(renderer, 32); }});
    tests.push_back({"VI_TOP_FRAMEBUFFER_REGISTER",
                     []() {
                         Bus::write32(VI_BASE + 0x1C, 0x00123456);
                         return Bus::read32(VI_BASE + 0x1C) == 0x00123456;
                     },
                     [](SDL_Renderer *renderer) {
                         clear(renderer, 0, 0, 0);
                         setColor(renderer, 255, 255, 255);
                         fill(renderer, 0, 0, WIDTH, HEIGHT / 2.0f);
                     }});
    tests.push_back({"VI_BOTTOM_FRAMEBUFFER_REGISTER",
                     []() {
                         Bus::write32(VI_BASE + 0x24, 0x00765432);
                         return Bus::read32(VI_BASE + 0x24) == 0x00765432;
                     },
                     [](SDL_Renderer *renderer) {
                         clear(renderer, 255, 255, 255);
                         setColor(renderer, 0, 0, 0);
                         fill(renderer, 0, HEIGHT / 2.0f, WIDTH, HEIGHT / 2.0f);
                     }});
    tests.push_back(
        {"VI_BEAM_SCANLINE_ADVANCE",
         [&device]() {
             uint32_t before = Bus::read16(
                 VI_BASE + static_cast<uint32_t>(VIRegister::VerticalBeam));
             device.vi->onScanLine();
             uint32_t after = Bus::read16(
                 VI_BASE + static_cast<uint32_t>(VIRegister::VerticalBeam));
             return after == before + 1;
         },
         [](SDL_Renderer *renderer) {
             clear(renderer, 0, 0, 0);
             for (int y = 0; y < HEIGHT; y += 2) {
                 setColor(renderer, 255, 255, 255);
                 SDL_RenderLine(renderer, 0, static_cast<float>(y), WIDTH,
                                static_cast<float>(y));
             }
         }});
    tests.push_back(
        {"VI_BEAM_FRAME_WRAP",
         [&device]() {
             uint32_t current = Bus::read16(
                 VI_BASE + static_cast<uint32_t>(VIRegister::VerticalBeam));
             for (uint32_t line = current; line <= VI_TOTAL_LINES; ++line)
                 device.vi->onScanLine();
             return Bus::read16(VI_BASE + static_cast<uint32_t>(
                                              VIRegister::VerticalBeam)) == 1;
         },
         [](SDL_Renderer *renderer) {
             for (int y = 0; y < HEIGHT; ++y) {
                 uint8_t value = static_cast<uint8_t>(y * 255 / (HEIGHT - 1));
                 setColor(renderer, value, 255 - value, 128);
                 SDL_RenderLine(renderer, 0, static_cast<float>(y), WIDTH,
                                static_cast<float>(y));
             }
         }});
    tests.push_back(
        {"VI_SCANLINE_INTERRUPT_ASSERTION",
         [&device]() {
             uint32_t current = Bus::read16(
                 VI_BASE + static_cast<uint32_t>(VIRegister::VerticalBeam));
             uint32_t target = current + 1;
             Bus::write32(VI_BASE + 0x30, (1u << 28) | (target << 16) | 1);
             Bus::write32(PI_MMIO_BASE + 0x04,
                          1u << static_cast<uint32_t>(PIInterrupt::VI));
             device.vi->onScanLine();
             return device.pi->interruptPending() &&
                    (Bus::read32(VI_BASE + 0x30) & (1u << 31));
         },
         [](SDL_Renderer *renderer) {
             clear(renderer, 0, 0, 0);
             setColor(renderer, 255, 32, 32);
             fill(renderer, 0, HEIGHT / 2.0f - 2, WIDTH, 4);
         }});
    tests.push_back(
        {"VI_INTERRUPT_ACKNOWLEDGEMENT",
         [&device]() {
             uint32_t current = Bus::read16(
                 VI_BASE + static_cast<uint32_t>(VIRegister::VerticalBeam));
             Bus::write32(VI_BASE + 0x30,
                          (1u << 28) | ((current + 1) << 16) | 1);
             return !device.pi->interruptPending() &&
                    !(Bus::read32(VI_BASE + 0x30) & (1u << 31));
         },
         [](SDL_Renderer *renderer) {
             clear(renderer, 0, 0, 0);
             setColor(renderer, 64, 255, 128);
             fill(renderer, WIDTH / 2.0f - 80, HEIGHT / 2.0f - 80, 160, 160);
         }});
}
}

int runGraphicsSuite() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Revolve Graphics Tests", WIDTH, HEIGHT, 0,
                                     &window, &renderer)) {
        std::cerr << "Graphics window creation failed: " << SDL_GetError()
                  << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    auto device = Device::createDevice();
    std::vector<GraphicsTest> tests;
    addHostTests(tests);
    addVITests(tests, *device);

    size_t passed = 0;
    size_t failed = 0;
    std::cout << "TAB = OK, ENTER = BAD\n\n";

    try {
        for (size_t index = 0; index < tests.size(); ++index) {
            const auto &test = tests[index];
            std::string title = "Revolve Graphics Tests - " + test.name;
            SDL_SetWindowTitle(window, title.c_str());

            bool automaticResult = test.validate();
            test.render(renderer);
            label(renderer, test.name);
            SDL_RenderPresent(renderer);

            std::cout << '[' << index + 1 << '/' << tests.size() << "] "
                      << std::left << std::setw(36) << test.name
                      << " automatic=" << (automaticResult ? "PASS" : "FAIL")
                      << "  TAB=OK ENTER=BAD" << std::flush;

            bool visualResult = waitForVerdict();
            bool result = automaticResult && visualResult;
            if (result) {
                ++passed;
                std::cout << "  \033[32m[PASSED]\033[0m\n";
            } else {
                ++failed;
                std::cout << "  \033[31m[FAILED]\033[0m\n";
            }
        }
    } catch (const std::exception &error) {
        std::cerr << "\nGraphics suite aborted: " << error.what() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << '\n'
              << passed << '/' << tests.size() << " graphics tests passed";
    if (failed)
        std::cout << ", " << failed << " failed";
    std::cout << ".\n";
    return failed == 0 ? 0 : 1;
}
