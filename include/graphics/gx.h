#ifndef GX_H
#define GX_H

#include <array>
#include <cstdint>

struct GXVec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct GXVec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct GXColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

struct GXVertex {
    uint8_t positionMatrixIndex = 0;
    std::array<uint8_t, 8> texMatrixIndices{};

    GXVec3 position{};
    GXVec3 normal{};

    GXColor color0{};
    GXColor color1{};

    std::array<GXVec2, 8> texCoords{};
};

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

  private:
    void decodeVCDLo(uint32_t value);
    void decodeVCDHi(uint32_t value);

    std::array<uint32_t, 256> registers{};

    GXVertexDescriptor vcd{};

    std::array<GXVAT, 8> vat{};

    std::array<uint32_t, 16> arrayBases{};
    std::array<uint32_t, 16> arrayStrides{};
};

struct GXXFState {
    std::array<uint32_t, 0x2000> registers{};
};

struct GXBPState {
    std::array<uint32_t, 256> registers{};
};

struct GXState {
    GXCommandProcessorState cp{};
    GXXFState xf{};
    GXBPState bp{};
};

class GX {
  public:
    void run();
    void processCommand();

    void initializeFifoReader();

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
    GXColor readDirectColor(uint8_t vat, uint32_t colorIndex);
    GXVec2 readDirectTexCoord(uint8_t vat, uint32_t index);
    GXVertex readVertex(uint8_t vat);

    GXFifoReader reader{};
    GXState state{};
};

#endif