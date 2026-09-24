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

    GXVec3 operator+(const GXVec3 &other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    GXVec3 operator-(const GXVec3 &other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    GXVec3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    GXVec3 operator/(float scalar) const {
        return {x / scalar, y / scalar, z / scalar};
    }
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
    float x, y, z, w;

    // Raster 1
    float r, g, b, a;

    // Raster 2
    float r1, g1, b1, a1;

    float u0, v0;
    float u1, v1;
    float u2, v2;
    float u3, v3;
    float u4, v4;
    float u5, v5;
    float u6, v6;
    float u7, v7;
};

struct GXMatrix3x4 {
    float m[3][4]{};
};

struct GXMatrix3x3 {
    float m[3][3]{};
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

enum class GXTLUTFormat : uint8_t { IA8 = 0x0, RGB565 = 0x1, RGB5A3 = 0x2 };

struct GXTLUTState {
    uint32_t address = 0;
    GXTLUTFormat format = GXTLUTFormat::IA8;
};

enum class GXTextureFormat : uint8_t {
    I4 = 0x0,
    I8 = 0x1,
    IA4 = 0x2,
    IA8 = 0x3,
    RGB565 = 0x4,
    RGB5A3 = 0x5,
    RGBA8 = 0x6,
    C4 = 0x8,
    C8 = 0x9,
    C14X2 = 0xA,
    CMPR = 0xE
};

struct GXTextureState {
    uint32_t address = 0;

    uint16_t width = 1;
    uint16_t height = 1;

    GXTextureFormat format = GXTextureFormat::RGBA8;

    uint8_t wrapS = 0;
    uint8_t wrapT = 0;

    uint8_t minFilter = 0;
    uint8_t magFilter = 0;

    float lodBias = 0.0f;

    bool valid = false;

    GXTLUTState tlut{};

    uint32_t image1 = 0;
    uint32_t image2 = 0;
};

struct GXDecodedTexture {
    uint32_t width = 0;
    uint32_t height = 0;

    std::vector<uint8_t> rgba;
};

struct GXTevOrder {
    uint8_t texCoord = 0;
    uint8_t texMap = 0;
    uint8_t colorChannel = 0;

    bool textureEnabled = false;
};

enum class GXTevColorArg : uint8_t {
    PrevColor = 0,
    PrevAlpha = 1,

    Color0 = 2,
    Alpha0 = 3,
    Color1 = 4,
    Alpha1 = 5,
    Color2 = 6,
    Alpha2 = 7,

    TexColor = 8,
    TexAlpha = 9,
    RasColor = 10,
    RasAlpha = 11,

    One = 12,
    Half = 13,
    Konst = 14,
    Zero = 15
};

enum class GXTevAlphaArg : uint8_t {
    PrevAlpha = 0,

    Alpha0 = 1,
    Alpha1 = 2,
    Alpha2 = 3,

    TexAlpha = 4,
    RasAlpha = 5,
    Konst = 6,
    Zero = 7
};

enum class GXTevBias : uint8_t {
    Zero = 0,
    AddHalf = 1,
    SubHalf = 2,
    Compare = 3
};

enum class GXTevOp : uint8_t {
    Add = 0,
    Sub = 1,
};

enum class GXTevScale : uint8_t {
    Scale1 = 0,
    Scale2 = 1,
    Scale4 = 2,
    Divide2 = 3
};

enum class GXTevOutput : uint8_t {
    Prev = 0,
    Color0 = 1,
    Color1 = 2,
    Color2 = 3
};

struct GXTevColorCombiner {
    GXTevColorArg a = GXTevColorArg::Zero;
    GXTevColorArg b = GXTevColorArg::Zero;
    GXTevColorArg c = GXTevColorArg::Zero;
    GXTevColorArg d = GXTevColorArg::Zero;

    GXTevBias bias = GXTevBias::Zero;
    GXTevOp op = GXTevOp::Add;
    bool clamp = true;
    GXTevScale scale = GXTevScale::Scale1;
    GXTevOutput output = GXTevOutput::Prev;
};

struct GXTevAlphaCombiner {
    GXTevAlphaArg a = GXTevAlphaArg::Zero;
    GXTevAlphaArg b = GXTevAlphaArg::Zero;
    GXTevAlphaArg c = GXTevAlphaArg::Zero;
    GXTevAlphaArg d = GXTevAlphaArg::Zero;

    GXTevBias bias = GXTevBias::Zero;
    GXTevOp op = GXTevOp::Add;
    bool clamp = true;
    GXTevScale scale = GXTevScale::Scale1;
    GXTevOutput output = GXTevOutput::Prev;

    uint8_t rasterSwap = 0;
    uint8_t textureSwap = 0;
};

struct GXTevStage {
    GXTevOrder order{};
    GXTevColorCombiner color{};
    GXTevAlphaCombiner alpha{};
};

enum class GXTexProjection : uint8_t { ST = 0, STQ = 1 };

enum class GXTexInputForm : uint8_t { AB11 = 0, ABC1 = 1 };

enum class GXTexGenType : uint8_t {
    Regular = 0,
    EmbossMap = 1,
    Color0 = 2,
    Color1 = 3,
};

enum class GXTexSource : uint8_t {
    Position = 0,
    Normal = 1,
    Colors = 2,
    BinormalT = 3,
    BinormalB = 4,

    Tex0 = 5,
    Tex1 = 6,
    Tex2 = 7,
    Tex3 = 8,
    Tex4 = 9,
    Tex5 = 10,
    Tex6 = 11,
    Tex7 = 12,
};

struct GXTexGenState {
    GXTexProjection projection = GXTexProjection::ST;
    GXTexInputForm inputForm = GXTexInputForm::AB11;
    GXTexGenType type = GXTexGenType::Regular;
    GXTexSource source = GXTexSource::Tex0;

    uint8_t embossSource = 0;
    uint8_t embossLight = 0;
};

struct GXPostTexMatrixState {
    uint8_t index = 0;
    bool normalize = false;
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

    XFIndexedLoadA = 0x20,
    XFIndexedLoadB = 0x28,
    XFIndexedLoadC = 0x30,
    XFIndexedLoadD = 0x38,

    CallDisplayList = 0x40,
    InvalidateVertexCache = 0x48,

    BPLoad = 0x61,
};

enum class GXCommandSource { FIFO, DisplayList };

struct GXDisplayListReader {
    uint32_t address = 0;
    uint32_t remaining = 0;
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

struct GXLight {
    GXColor color{};

    GXVec3 cosAttenuation{};
    GXVec3 distAttenuation{};

    GXVec3 position{};
    GXVec3 direction{};
};

struct GXLightingChannel {
    uint32_t colorControl = 0x401;
    uint32_t alphaControl = 0x401;
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

    inline uint32_t getTextureMatrixIndex(uint32_t texGen) const {
        if (texGen >= textureMatrixIndices.size())
            return 0;

        return textureMatrixIndices[texGen];
    }

  private:
    void decodeVCDLo(uint32_t value);
    void decodeVCDHi(uint32_t value);

    void decodeMatrixIndexA(uint32_t value);
    void decodeMatrixIndexB(uint32_t value);

    std::array<uint32_t, 256> registers{};

    GXVertexDescriptor vcd{};

    std::array<GXVAT, 8> vat{};

    std::array<uint32_t, 16> arrayBases{};
    std::array<uint32_t, 16> arrayStrides{};

    uint32_t positionMatrixIndex = 0;
    std::array<uint8_t, 8> textureMatrixIndices{};
};

struct GXXFState {
    std::array<uint32_t, 0x2000> registers{};

    std::array<float, 1024> matrixMemory{};

    std::array<float, 96> normalMatrixMemory{};

    std::array<float, 256> postMatrices{};

    uint8_t numTexGens = 0;

    GXProjection projection{};
    GXViewport viewport{};

    std::array<GXTexGenState, 8> texGens{};
    std::array<GXPostTexMatrixState, 8> postTexMatrices{};
    std::array<GXLight, 8> lights{};

    bool dualTexTransform = false;

    uint8_t numColorChannels = 0;

    std::array<GXColor, 2> ambientColors{};
    std::array<GXColor, 2> materialColors{GXColor{1.0f, 1.0f, 1.0f, 1.0f},
                                          GXColor{1.0f, 1.0f, 1.0f, 1.0f}};

    std::array<GXLightingChannel, 2> lightingChannels{};
};

struct GXBPCopyState {
    uint16_t sourceX = 0;
    uint16_t sourceY = 0;

    uint16_t sourceWidth = 0;
    uint16_t sourceHeight = 0;

    uint32_t xfbAddress = 0;
    uint16_t xfbStride = 0;

    GXColor clearColor{};
    uint32_t clearDepth = 0xFFFFFF;

    bool clearAfterCopy = false;
    bool copyToXfb = false;
};

static constexpr uint32_t BP_COPY_CLEAR_MASK = 1u << 11;

static constexpr uint32_t BP_COPY_TO_XFB_MASK = 1u << 14;

enum class GXAlphaLogic : uint8_t { And = 0, Or = 1, Xor = 2, Xnor = 3 };

struct GXAlphaTestState {
    uint8_t ref0 = 0;
    uint8_t ref1 = 0;

    opal::CompareOp comp0 = opal::CompareOp::Always;
    opal::CompareOp comp1 = opal::CompareOp::Always;

    GXAlphaLogic logic = GXAlphaLogic::And;
};

struct GXRasterState {
    bool depthTest = false;
    bool depthWrite = false;

    opal::CompareOp depthCompare = opal::CompareOp::Less;
    opal::CullMode cullMode = opal::CullMode::None;

    bool blendEnabled = false;
    bool subtractBlend = false;

    opal::BlendFunc srcBlend = opal::BlendFunc::One;
    opal::BlendFunc dstBlend = opal::BlendFunc::Zero;

    bool colorWrite = true;
    bool alphaWrite = true;

    bool logicOpEnabled = false;
    uint8_t logicOp = 0;

    opal::FrontFace frontFace = opal::FrontFace::Clockwise;

    uint16_t scissorX = 0;
    uint16_t scissorY = 0;
    uint16_t scissorWidth = 640;
    uint16_t scissorHeight = 528;
};

struct GXScissorState {
    uint16_t left = 342;
    uint16_t top = 342;

    uint16_t right = 342 + 639;
    uint16_t bottom = 342 + 527;

    uint16_t offsetXHalf = 171;
    uint16_t offsetYHalf = 171;
};

struct GXBPState {
    std::array<uint32_t, 256> registers{};
    GXBPCopyState copy{};
    GXRasterState raster{};
    GXScissorState scissor{};

    GXAlphaTestState alphaTest{};

    std::array<GXTextureState, 8> textures{};
    std::array<GXTevStage, 16> tevStages{};

    uint32_t tevStageCount = 1;
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

static constexpr uint32_t EFB_WIDTH = 640;
static constexpr uint32_t EFB_HEIGHT = 528;

struct GXXFB {
    std::shared_ptr<opal::Texture> texture;
    std::shared_ptr<opal::Framebuffer> framebuffer;
    std::shared_ptr<opal::RenderPass> renderPass;

    uint32_t address = 0;
    uint32_t stride = 0;

    uint32_t width = 0;
    uint32_t height = 0;

    bool valid = false;
};

namespace gx {
static inline int textureUnitFromBP(uint8_t reg, uint8_t base0, uint8_t base4) {
    if (reg >= base0 && reg < base0 + 4)
        return reg - base0;

    if (reg >= base4 && reg < base4 + 4)
        return 4 + (reg - base4);

    return -1;
}

static inline GXColor decodeXFColor(uint32_t value) {
    GXColor color{};

    color.r = static_cast<float>((value >> 24) & 0xFF) / 255.0f;
    color.g = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
    color.b = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
    color.a = static_cast<float>(value & 0xFF) / 255.0f;

    return color;
}

static inline uint8_t getGXLightMask(uint32_t control) {
    const uint8_t low = static_cast<uint8_t>((control >> 2) & 0xF);
    const uint8_t high = static_cast<uint8_t>((control >> 11) & 0xF);

    return low | (high << 4);
}

static inline float dot(const GXVec3 &a, const GXVec3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float length(const GXVec3 &v) { return std::sqrt(dot(v, v)); }

static inline GXVec3 normalize(const GXVec3 &v) {
    const float len = length(v);

    GXVec3 result = v;

    if (len > 0.0f) {
        result.x /= len;
        result.y /= len;
        result.z /= len;
    }

    return result;
}

} // namespace gx

class GXRenderer {
  public:
    void initialize();

    void drawTriangle(const GXRenderVertex &a, const GXRenderVertex &b,
                      const GXRenderVertex &c);

    void flushEFB();

    void copyEFBToXFB(const GXBPCopyState &copy);

    void clearEFB(const GXColor &color, uint32_t depth);

    void presentXFB();

    void setRasterState(const GXRasterState &state);

    void setAlphaTestState(const GXAlphaTestState &state);

    SDL_Window *window = nullptr;

    void setTexture(uint32_t unit, const GXDecodedTexture &texture,
                    const GXTextureState &state);

  private:
    void rebuildGXPipeline();

    void createEFB();
    void createPresentPipeline();

    void ensureXFB(uint32_t width, uint32_t height);

    std::shared_ptr<opal::Device> device;

    std::shared_ptr<opal::Framebuffer> displayFramebuffer;
    std::shared_ptr<opal::RenderPass> displayRenderPass;

    std::shared_ptr<opal::Pipeline> gxPipeline;
    std::shared_ptr<opal::ShaderProgram> gxShaderProgram;

    std::shared_ptr<opal::Texture> efbColor;
    std::shared_ptr<opal::Texture> efbDepth;
    std::shared_ptr<opal::Framebuffer> efbFramebuffer;
    std::shared_ptr<opal::RenderPass> efbRenderPass;

    GXXFB xfb{};

    std::shared_ptr<opal::Pipeline> presentPipeline;

    std::shared_ptr<opal::Buffer> fullscreenBuffer;
    std::shared_ptr<opal::DrawingState> fullscreenDrawingState;

    std::vector<GXRenderVertex> vertices;

    bool rasterStateDirty = true;
    GXRasterState currentRasterState{};

    std::vector<opal::VertexAttribute> gxAttributes;

    opal::VertexBinding gxBinding{};

    GXAlphaTestState currentAlphaTest{};

    std::array<std::shared_ptr<opal::Texture>, 8> boundTextures{};
    std::array<bool, 8> textureValid{};

    inline opal::TextureWrapMode decodeWrapMode(uint8_t mode) const {
        switch (mode) {
        case 0:
            return opal::TextureWrapMode::ClampToEdge;
        case 1:
            return opal::TextureWrapMode::Repeat;
        case 2:
            return opal::TextureWrapMode::MirroredRepeat;
        default:
            return opal::TextureWrapMode::ClampToEdge;
        }
    }

    inline opal::TextureFilterMode decodeMagFilter(uint8_t mode) const {
        return mode == 0 ? opal::TextureFilterMode::Nearest
                         : opal::TextureFilterMode::Linear;
    }

    inline opal::TextureFilterMode decodeMinFilter(uint8_t mode) const {
        switch (mode) {
        case 0:
            return opal::TextureFilterMode::Nearest;
        case 1:
            return opal::TextureFilterMode::Linear;
        case 2:
        case 3:
        case 4:
        case 5:
            return opal::TextureFilterMode::LinearMipmapLinear;
        default:
            return opal::TextureFilterMode::Linear;
        }
    }

    void applyTevState(std::shared_ptr<opal::Pipeline> &pipeline,
                       uint8_t stage);
};

class GX {
  public:
    void initialize();
    void run();
    void processCommand();

    void initializeFifoReader();
    void onFifoBytesAvailable(uint32_t bytes);

    std::shared_ptr<GXRenderer> renderer = std::make_shared<GXRenderer>();

    static inline uint8_t expand3(uint8_t v) {
        return static_cast<uint8_t>((v << 5) | (v << 2) | (v >> 1));
    }

    static inline uint8_t expand4(uint8_t v) {
        return static_cast<uint8_t>((v << 4) | v);
    }

    static inline uint8_t expand5(uint8_t v) {
        return static_cast<uint8_t>((v << 3) | (v >> 2));
    }

    static inline uint8_t expand6(uint8_t v) {
        return static_cast<uint8_t>((v << 2) | (v >> 4));
    }

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

    static inline void writePixel(GXDecodedTexture &result, uint32_t x,
                                  uint32_t y, uint8_t r, uint8_t g, uint8_t b,
                                  uint8_t a) {
        if (x >= result.width || y >= result.height)
            return;

        const size_t dst = (static_cast<size_t>(y) * result.width + x) * 4;

        result.rgba[dst + 0] = r;
        result.rgba[dst + 1] = g;
        result.rgba[dst + 2] = b;
        result.rgba[dst + 3] = a;
    }

    static inline void writeGXColor(GXDecodedTexture &result, uint32_t x,
                                    uint32_t y, const GXColor &color) {
        writePixel(result, x, y, static_cast<uint8_t>(color.r * 255.0f),
                   static_cast<uint8_t>(color.g * 255.0f),
                   static_cast<uint8_t>(color.b * 255.0f),
                   static_cast<uint8_t>(color.a * 255.0f));
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

    void writeBP(uint8_t reg, uint32_t value);
    void executeEfbCopy();

    void updateScissorState();

    opal::BlendFunc decodeGXSrcBlendFactor(uint32_t factor) const;
    opal::BlendFunc decodeGXDstBlendFactor(uint32_t factor) const;
    opal::CompareOp decodeGXCompare(uint32_t value) const;

    GXDecodedTexture decodeTexture(const GXTextureState &state) const;

    GXDecodedTexture decodeTextureI4(uint32_t address, uint16_t width,
                                     uint16_t height) const;
    GXDecodedTexture decodeTextureI8(uint32_t address, uint16_t width,
                                     uint16_t height) const;
    GXDecodedTexture decodeTextureIA4(uint32_t address, uint16_t width,
                                      uint16_t height) const;
    GXDecodedTexture decodeTextureIA8(uint32_t address, uint16_t width,
                                      uint16_t height) const;
    GXDecodedTexture decodeTextureRGB565(uint32_t address, uint16_t width,
                                         uint16_t height) const;
    GXDecodedTexture decodeTextureRGB5A3(uint32_t address, uint16_t width,
                                         uint16_t height) const;
    GXDecodedTexture decodeTextureRGBA8(uint32_t address, uint16_t width,
                                        uint16_t height) const;
    GXDecodedTexture decodeTextureC4(uint32_t address, uint16_t width,
                                     uint16_t height,
                                     const GXTLUTState &tlut) const;
    GXDecodedTexture decodeTextureC8(uint32_t address, uint16_t width,
                                     uint16_t height,
                                     const GXTLUTState &tlut) const;
    GXDecodedTexture decodeTextureC14X2(uint32_t address, uint16_t width,
                                        uint16_t height,
                                        const GXTLUTState &tlut) const;
    GXDecodedTexture decodeTextureCMPR(uint32_t address, uint16_t width,
                                       uint16_t height) const;

    void decodeCMPRSubBlock(GXDecodedTexture &result, uint32_t &address,
                            uint32_t originX, uint32_t originY) const;

    GXColor decodeTLUTEntry(uint32_t address, GXTLUTFormat format) const;

    void decodeTextureMode0(uint32_t unit, uint32_t value);
    void decodeTextureImage0(uint32_t unit, uint32_t value);
    void decodeTextureImage3(uint32_t unit, uint32_t value);
    void decodeTextureImage1(uint32_t unit, uint32_t value);
    void decodeTextureImage2(uint32_t unit, uint32_t value);
    void decodeTextureTLUT(uint32_t unit, uint32_t value);

    void updateTextureUnit(uint32_t unit);

    void decodeTevOrder(uint8_t reg, uint32_t value);
    void decodeTevOrderStage(uint32_t stage, uint32_t raw);

    void decodeTevColorCombiner(uint32_t stage, uint32_t value);
    void decodeTevAlphaCombiner(uint32_t stage, uint32_t value);

    uint32_t getTextureMatrixIndex(const GXVertex &vertex,
                                   uint32_t texGen) const;

    GXVec4 getTexGenSource(const GXVertex &vertex, GXTexSource source) const;
    GXVec3 applyTextureMatrix(const GXVec4 &v, uint32_t matrixIndex,
                              GXTexProjection projection) const;
    GXVec3 applyPostTextureMatrix(GXVec3 tex, uint32_t texGen) const;

    GXVec3 generateTexCoord(const GXVertex &vertex, uint32_t index) const;

    void processIndexedXF(uint8_t command);

    void processCallDisplayList();
    void processDisplayList(uint32_t address, uint32_t size);

    GXMatrix3x3 getNormalMatrix(uint32_t matrixIndex) const;
    GXVec3 transformNormal(const GXVertex &vertex, uint32_t matrixIndex) const;

    GXColor calculateLight(const GXLight &light, uint32_t control,
                           const GXVec3 &position, const GXVec3 &normal) const;

    GXColor calculateLightingChannel(const GXVertex &vertex, uint32_t channel,
                                     const GXVec3 &viewPosition,
                                     const GXVec3 &normal) const;

    GXColor getVertexColor(const GXVertex &vertex, uint32_t channel) const;

    GXFifoReader reader{};

    GXCommandSource commandSource = GXCommandSource::FIFO;
    GXDisplayListReader displayListReader{};

    uint32_t displayListDepth = 0;

    GXState state{};

    friend class GXRenderer;
};

#endif
