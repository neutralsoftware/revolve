#ifndef GX_H
#define GX_H

#include "SDL3/SDL_video.h"
#include "core/memory.h"
#include "opal/opal.h"
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

struct GXVec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct GXVec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct GXVec4 {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

struct GXColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

struct GXRenderVertex {
    float x;
    float y;
    float z;
    float w;

    float r;
    float g;
    float b;
    float a;

    float u;
    float v;
};

struct GXMatrix3x4 {
    float m[3][4]{};
};

struct GXProjection {
    float values[6]{};
    uint32_t mode = 0;
};

struct GXVertex {
    uint8_t positionMatrixIndex = 0;
    std::array<uint8_t, 8> texMatrixIndices{};

    GXVec3 position{};

    GXVec3 normal{};
    GXVec3 binormal{};
    GXVec3 tangent{};

    GXColor color0{};
    GXColor color1{};

    std::array<GXVec2, 8> texCoords{};
};

struct GXViewport {
    float xScale = 0.0f;
    float yScale = 0.0f;

    float zRange = 0.0f;

    float xOrigin = 0.0f;
    float yOrigin = 0.0f;

    float farZ = 0.0f;
};

enum class GXArrayAttribute : uint8_t {
    Position = 9,
    Normal = 10,
    Color0 = 11,
    Color1 = 12,

    Tex0 = 13,
    Tex1 = 14,
    Tex2 = 15,
    Tex3 = 16,
    Tex4 = 17,
    Tex5 = 18,
    Tex6 = 19,
    Tex7 = 20
};

static constexpr uint32_t arrayIndex(GXArrayAttribute attr) {
    return static_cast<uint32_t>(attr) - 9;
}

struct GXFifoReader {
    uint32_t cursor = 0;
    uint32_t bytesIntoBlock = 0;
    uint32_t availableBytes = 0;
};

enum class GXCommand : uint8_t {
    NOP = 0x00,
    CPLoad = 0x08,
    XFLoad = 0x10,
    BPLoad = 0x61
};

enum class GXCPRegister : uint8_t {
    MatrixIndexA = 0x30,
    MatrixIndexB = 0x40,

    VCDLo = 0x50,
    VCDHi = 0x60,

    VAT_A0 = 0x70,
    VAT_B0 = 0x80,
    VAT_C0 = 0x90,

    ArrayBase0 = 0xA0,
    ArrayStride0 = 0xB0,
};

enum class GXVertexAttributeMode : uint8_t {
    None = 0,
    Direct = 1,
    Index8 = 2,
    Index16 = 3
};

struct GXVertexDescriptor {
    bool positionMatrixIndex = false;

    std::array<bool, 8> texMatrixIndex{};

    GXVertexAttributeMode position = GXVertexAttributeMode::None;
    GXVertexAttributeMode normal = GXVertexAttributeMode::None;

    GXVertexAttributeMode color0 = GXVertexAttributeMode::None;
    GXVertexAttributeMode color1 = GXVertexAttributeMode::None;

    std::array<GXVertexAttributeMode, 8> texCoord{};
};

struct GXVAT {
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;
};

enum class GXComponentFormat : uint8_t {
    U8 = 0,
    S8 = 1,
    U16 = 2,
    S16 = 3,
    F32 = 4
};

struct GXPositionFormat {
    uint32_t components = 3;
    GXComponentFormat format = GXComponentFormat::F32;
    uint8_t fractionalBits = 0;
};

struct GXNormalFormat {
    uint32_t vectors = 1;
    GXComponentFormat format = GXComponentFormat::F32;
    bool index3 = false;
};

enum class GXColorFormat : uint8_t {
    RGB565 = 0,
    RGB8 = 1,
    RGBX8 = 2,
    RGBA4 = 3,
    RGBA6 = 4,
    RGBA8 = 5
};

struct GXColorAttributeFormat {
    GXColorFormat format = GXColorFormat::RGBA8;
};

struct GXTexCoordFormat {
    uint32_t components = 2;
    GXComponentFormat format = GXComponentFormat::F32;
    uint8_t fractionalBits = 0;
};

class GXCommandProcessorState {
  public:
    void write(uint8_t reg, uint32_t value);

    inline const GXVertexDescriptor &getVCD() const { return vcd; }

    inline const GXVAT &getVAT(uint8_t index) const { return vat[index]; }
    inline uint32_t getArrayBase(uint8_t index) const {
        return arrayBases[index];
    }

    inline uint32_t getArrayStride(uint8_t index) const {
        return arrayStrides[index];
    }

    inline static GXVertexAttributeMode decodeMode(uint32_t value) {
        return static_cast<GXVertexAttributeMode>(value & 0x3);
    }

    inline static uint32_t getAttributeIndexSize(GXVertexAttributeMode mode,
                                                 uint32_t directSize) {
        switch (mode) {
        case GXVertexAttributeMode::None:
            return 0;
        case GXVertexAttributeMode::Direct:
            return directSize;
        case GXVertexAttributeMode::Index8:
            return 1;
        case GXVertexAttributeMode::Index16:
            return 2;
        default:
            return 0;
        }
    }

    inline static uint32_t componentSize(GXComponentFormat format) {
        switch (format) {
        case GXComponentFormat::U8:
            return 1;
        case GXComponentFormat::S8:
            return 1;
        case GXComponentFormat::U16:
            return 2;
        case GXComponentFormat::S16:
            return 2;
        case GXComponentFormat::F32:
            return 4;
        default:
            return 0;
        }
    }

    inline static uint32_t colorSize(GXColorFormat format) {
        switch (format) {
        case GXColorFormat::RGB565:
            return 2;
        case GXColorFormat::RGB8:
            return 3;
        case GXColorFormat::RGBX8:
            return 4;
        case GXColorFormat::RGBA4:
            return 2;
        case GXColorFormat::RGBA6:
            return 3;
        case GXColorFormat::RGBA8:
            return 4;
        default:
            return 0;
        }
    }

    GXPositionFormat getPositionFormat(uint8_t vatIndex) const;
    uint32_t getDirectPositionSize(uint8_t vatIndex) const;

    GXNormalFormat getNormalFormat(uint8_t vatIndex) const;
    uint32_t getDirectNormalSize(uint8_t vatIndex) const;

    GXColorAttributeFormat getColorFormat(uint8_t vatIndex,
                                          uint32_t colorIndex) const;
    uint32_t getDirectColorSize(uint8_t vatIndex, uint32_t colorIndex) const;

    GXTexCoordFormat getTexCoordFormat(uint8_t vatIndex,
                                       uint32_t texIndex) const;
    uint32_t getDirectTexCoordSize(uint8_t vatIndex, uint32_t texIndex) const;

    uint32_t getVertexSize(uint8_t vatIndex) const;

    inline uint32_t getPositionMatrixIndex() const {
        return positionMatrixIndex;
    }

  private:
    void decodeVCDLo(uint32_t value);
    void decodeVCDHi(uint32_t value);

    std::array<uint32_t, 256> registers{};

    GXVertexDescriptor vcd{};

    std::array<GXVAT, 8> vat{};

    std::array<uint32_t, 16> arrayBases{};
    std::array<uint32_t, 16> arrayStrides{};

    uint32_t positionMatrixIndex = 0;
};

struct GXXFState {
    std::array<uint32_t, 0x2000> registers{};

    std::array<float, 1024> matrixMemory{};

    GXProjection projection{};
    GXViewport viewport{};
};

struct GXBPState {
    std::array<uint32_t, 256> registers{};
};

struct GXState {
    GXCommandProcessorState cp{};
    GXXFState xf{};
    GXBPState bp{};
};

enum class GXPrimitive : uint8_t {
    Quads = 0x80,
    Triangles = 0x90,
    TriangleStrip = 0x98,
    TriangleFan = 0xA0,
    Lines = 0xA8,
    LineStrip = 0xB0,
    Points = 0xB8
};

struct GXTriangle {
    GXVertex a;
    GXVertex b;
    GXVertex c;
};

struct GXLine {
    GXVertex a;
    GXVertex b;
};

class GXRenderer {
  public:
    void initialize();

    void drawTriangle(const GXRenderVertex &a, const GXRenderVertex &b,
                      const GXRenderVertex &c);

    SDL_Window *window = nullptr;

    void finishGXBatch();

    void present();

  private:
    void uploadVertices();

    std::shared_ptr<opal::Device> device;

    std::shared_ptr<opal::Pipeline> pipeline;
    std::shared_ptr<opal::ShaderProgram> shaderProgram;

    std::shared_ptr<opal::Buffer> vertexBuffer;
    std::shared_ptr<opal::DrawingState> drawingState;

    std::shared_ptr<opal::RenderPass> renderPass;
    std::shared_ptr<opal::Framebuffer> framebuffer;

    std::shared_ptr<opal::CommandBuffer> commandBuffer;

    std::vector<GXRenderVertex> pendingVertices;
    std::vector<GXRenderVertex> displayVertices;
};

class GX {
  public:
    void initialize();
    void run();
    void processCommand();

    void initializeFifoReader();
    void onFifoBytesAvailable(uint32_t bytes);

    std::shared_ptr<GXRenderer> renderer = std::make_shared<GXRenderer>();

  private:
    uint8_t read8();
    uint16_t read16();
    uint32_t read32();

    void processCPLoad();
    void processXFLoad();
    void processBPLoad();

    void processPrimitive(uint8_t command);

    float readComponent(GXComponentFormat format, uint8_t fractionalBits);

    GXVec3 readDirectPosition(uint8_t vatIndex);
    GXVec3 readDirectNormal(uint8_t vat);
    void readDirectNBT(uint8_t vat, GXVertex &vertex);
    GXColor readDirectColor(uint8_t vat, uint32_t colorIndex);
    GXVec2 readDirectTexCoord(uint8_t vat, uint32_t index);
    GXVertex readVertex(uint8_t vat);

    uint32_t readAttributeIndex(GXVertexAttributeMode mode);
    uint32_t getIndexedAddress(GXArrayAttribute attr, uint32_t index) const;

    inline uint8_t readMemory8(uint32_t &address) {
        return Bus::readPhysical8(address++);
    }

    inline uint16_t readMemory16(uint32_t &address) {
        uint16_t hi = static_cast<uint16_t>(Bus::readPhysical8(address++)) << 8;

        uint16_t lo = Bus::readPhysical8(address++);

        return hi | lo;
    }

    inline uint32_t readMemory32(uint32_t &address) {
        uint32_t hi = static_cast<uint32_t>(readMemory16(address)) << 16;

        uint32_t lo = readMemory16(address);

        return hi | lo;
    }

    float readMemoryComponent(uint32_t &address, GXComponentFormat format,
                              uint8_t fractionalBits);

    GXVec3 readIndexedPosition(uint8_t vat, uint32_t index);
    GXVec3 readIndexedNormal(uint8_t vat, uint32_t index);
    GXVec2 readIndexedTexCoord(uint8_t vat, uint32_t texIndex, uint32_t index);
    GXColor readIndexedColor(uint8_t vat, uint32_t colorIndex, uint32_t index);

    void emitTriangle(const GXVertex &a, const GXVertex &b, const GXVertex &c);
    void emitLine(const GXVertex &a, const GXVertex &b);
    void emitPoint(const GXVertex &point);

    void assemblePrimitive(GXPrimitive primitive,
                           const std::vector<GXVertex> &vertices);

    void writeXF(uint16_t address, uint32_t value);

    GXMatrix3x4 getPositionMatrix(uint32_t matrixIndex) const;
    uint32_t getVertexPositionMatrixIndex(const GXVertex &vertex) const;

    GXVec4 transformPosition(const GXVertex &vertex) const;
    GXVec4 projectPosition(const GXVec4 &v) const;
    GXVec3 clipToNDC(const GXVec4 &clip) const;
    GXVec3 viewportTransform(const GXVec3 &ndc) const;
    GXVec3 transformToScreen(const GXVertex &vertex) const;

    GXRenderVertex transformToRenderVertex(const GXVertex &vertex) const;

    GXFifoReader reader{};
    GXState state{};
};

#endif
