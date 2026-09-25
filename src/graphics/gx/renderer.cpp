#include "device.h"
#include "graphics/gx.h"
#include "graphics/shader.h"
#include "opal/opal.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace {
struct FullscreenVertex {
    float x;
    float y;
    float u;
    float v;
};

constexpr FullscreenVertex FULLSCREEN_VERTICES[] = {
    {-1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, -1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},   {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},   {-1.0f, 1.0f, 0.0f, 0.0f},
};

std::array<FullscreenVertex, 6> makeFullscreenVertices(float originX,
                                                       float originY,
                                                       float scaleX,
                                                       float scaleY) {
    const float left = originX;
    const float right = originX + scaleX;
    const float top = originY;
    const float bottom = originY + scaleY;
    return {{{-1.0f, -1.0f, left, bottom},
             {1.0f, -1.0f, right, bottom},
             {1.0f, 1.0f, right, top},
             {-1.0f, -1.0f, left, bottom},
             {1.0f, 1.0f, right, top},
             {-1.0f, 1.0f, left, top}}};
}
} // namespace

void GXRenderer::drawTriangle(const GXRenderVertex &a, const GXRenderVertex &b,
                              const GXRenderVertex &c) {
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
}

void GXRenderer::initialize() {
#ifdef METAL
    constexpr const char *vertexEntry = "main0";
    constexpr const char *fragmentEntry = "main0";
#else
    constexpr const char *vertexEntry = "vertexMain";
    constexpr const char *fragmentEntry = "fragmentMain";
#endif

    opal::ContextConfiguration config{
        .applicationName = "Revolve",
        .applicationVersion = {1, 0, 0},
        .createValidationLayers = true,
    };

    std::shared_ptr<opal::Context> context = opal::Context::create(config);
    window = context->makeWindow(1280, 720, "Revolve");
    device = opal::Device::acquire(context);
    displayFramebuffer = device->getDefaultFramebuffer();
    displayRenderPass = opal::RenderPass::create();
    displayRenderPass->setFramebuffer(displayFramebuffer);

    createEFB();

    auto gxVertexSource = opal::Shader::createFromSource(
        GX_VERTEX_SHADER, opal::ShaderType::Vertex);
    auto gxFragmentSource = opal::Shader::createFromSource(
        GX_FRAGMENT_SHADER, opal::ShaderType::Fragment);
    auto gxVertexShader =
        gxVertexSource->forFunction(vertexEntry, opal::ShaderType::Vertex);
    auto gxFragmentShader = gxFragmentSource->forFunction(
        fragmentEntry, opal::ShaderType::Fragment);
    gxVertexShader->compile();
    gxFragmentShader->compile();

    gxShaderProgram = opal::ShaderProgram::create();
    gxShaderProgram->attachShader(gxVertexShader);
    gxShaderProgram->attachShader(gxFragmentShader);
    gxShaderProgram->link();

    gxAttributes = {{.name = "inPosition",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, x),
                     .location = 0,
                     .normalized = false,
                     .size = 4,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inColor0",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, r),
                     .location = 1,
                     .normalized = false,
                     .size = 4,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inColor1",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, r1),
                     .location = 2,
                     .normalized = false,
                     .size = 4,
                     .stride = sizeof(GXRenderVertex)},

                    {.name = "inUV0",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u0),
                     .location = 3,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV1",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u1),
                     .location = 4,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV2",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u2),
                     .location = 5,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV3",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u3),
                     .location = 6,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV4",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u4),
                     .location = 7,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV5",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u5),
                     .location = 8,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV6",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u6),
                     .location = 9,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV7",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u7),
                     .location = 10,
                     .normalized = false,
                     .size = 2,
                     .stride = sizeof(GXRenderVertex)}};

    gxBinding.stride = sizeof(GXRenderVertex);
    gxBinding.inputRate = opal::VertexBindingInputRate::Vertex;

    rebuildGXPipeline();

    clearEFB({}, 0xFFFFFF);
    createPresentPipeline();
}

void GXRenderer::createEFB() {
    efbColor = opal::Texture::create(opal::TextureType::Texture2D,
                                     opal::TextureFormat::Rgba8, EFB_WIDTH,
                                     EFB_HEIGHT);
    efbDepth = opal::Texture::create(
        opal::TextureType::Texture2D, opal::TextureFormat::Depth24Stencil8,
        EFB_WIDTH, EFB_HEIGHT, opal::TextureDataFormat::DepthComponent);
    efbFramebuffer = opal::Framebuffer::create(EFB_WIDTH, EFB_HEIGHT);

    opal::Attachment attachment{};
    attachment.type = opal::Attachment::Type::Color;
    attachment.texture = efbColor;
    efbFramebuffer->addAttachment(attachment);

    opal::Attachment depthAttachment{};
    depthAttachment.type = opal::Attachment::Type::DepthStencil;
    depthAttachment.texture = efbDepth;
    efbFramebuffer->addAttachment(depthAttachment);

    efbRenderPass = opal::RenderPass::create();
    efbRenderPass->setFramebuffer(efbFramebuffer);
}

void GXRenderer::createPresentPipeline() {
#ifdef METAL
    constexpr const char *vertexEntry = "main0";
    constexpr const char *fragmentEntry = "main0";
#else
    constexpr const char *vertexEntry = "vertexMain";
    constexpr const char *fragmentEntry = "fragmentMain";
#endif

    auto vertexSource = opal::Shader::createFromSource(
        FULLSCREEN_VERTEX_SHADER, opal::ShaderType::Vertex);
    auto fragmentSource = opal::Shader::createFromSource(
        FULLSCREEN_FRAGMENT_SHADER, opal::ShaderType::Fragment);
    auto vertexShader =
        vertexSource->forFunction(vertexEntry, opal::ShaderType::Vertex);
    auto fragmentShader =
        fragmentSource->forFunction(fragmentEntry, opal::ShaderType::Fragment);
    vertexShader->compile();
    fragmentShader->compile();

    auto program = opal::ShaderProgram::create();
    program->attachShader(vertexShader);
    program->attachShader(fragmentShader);
    program->link();

    presentPipeline = opal::Pipeline::create();
    presentPipeline->setShaderProgram(program);
    presentPipeline->setPrimitiveStyle(opal::PrimitiveStyle::Triangles);
    presentPipeline->setRasterizerMode(opal::RasterizerMode::Fill);
    presentPipeline->setCullMode(opal::CullMode::None);
    presentPipeline->enableDepthTest(false);
    presentPipeline->enableDepthWrite(false);
    presentPipeline->enableBlending(false);

    std::vector<opal::VertexAttribute> attributes = {
        {.name = "inPosition",
         .type = opal::VertexAttributeType::Float,
         .offset = offsetof(FullscreenVertex, x),
         .location = 0,
         .normalized = false,
         .size = 2,
         .stride = sizeof(FullscreenVertex)},
        {.name = "inUV",
         .type = opal::VertexAttributeType::Float,
         .offset = offsetof(FullscreenVertex, u),
         .location = 1,
         .normalized = false,
         .size = 2,
         .stride = sizeof(FullscreenVertex)}};

    opal::VertexBinding binding{};
    binding.stride = sizeof(FullscreenVertex);
    binding.inputRate = opal::VertexBindingInputRate::Vertex;
    presentPipeline->setVertexAttributes(attributes, binding);
    presentPipeline->build();

    fullscreenBuffer = opal::Buffer::create(
        opal::BufferUsage::VertexBuffer, sizeof(FULLSCREEN_VERTICES),
        FULLSCREEN_VERTICES, opal::MemoryUsageType::CPUToGPU);
    fullscreenDrawingState = opal::DrawingState::create(fullscreenBuffer);
}

void GXRenderer::flushEFB() {
    if (vertices.empty())
        return;

    if (rasterStateDirty) {
        rebuildGXPipeline();
    }

    const size_t size = vertices.size() * sizeof(GXRenderVertex);
    auto vertexBuffer =
        opal::Buffer::create(opal::BufferUsage::VertexBuffer, size,
                             vertices.data(), opal::MemoryUsageType::CPUToGPU);
    auto drawingState = opal::DrawingState::create(vertexBuffer);
    auto commandBuffer = device->acquireCommandBuffer();
    commandBuffer->start();
    commandBuffer->beginPass(efbRenderPass);
    commandBuffer->setScissor(
        currentRasterState.scissorX, currentRasterState.scissorY,
        currentRasterState.scissorWidth, currentRasterState.scissorHeight);
    commandBuffer->bindPipeline(gxPipeline);
    commandBuffer->bindDrawingState(drawingState);

    gxPipeline->setUniform1f("alphaRef0", currentAlphaTest.ref0 / 255.0f);
    gxPipeline->setUniform1f("alphaRef1", currentAlphaTest.ref1 / 255.0f);
    gxPipeline->setUniform1i("alphaComp0",
                             static_cast<int>(currentAlphaTest.comp0));
    gxPipeline->setUniform1i("alphaComp1",
                             static_cast<int>(currentAlphaTest.comp1));
    gxPipeline->setUniform1i("alphaLogic",
                             static_cast<int>(currentAlphaTest.logic));
    const auto &bp = Device::globalDevice->gx.state.bp;
    gxPipeline->setUniform1i("tevStageCount",
                             static_cast<int>(bp.tevStageCount));
    gxPipeline->setUniform1i("indirectStageCount",
                             static_cast<int>(bp.indirectStageCount));

    gxPipeline->setUniform1i("destinationAlphaEnabled",
                             static_cast<int>(bp.destinationAlpha.enabled));
    gxPipeline->setUniform1f("destinationAlpha",
                             bp.destinationAlpha.alpha / 255.0f);

    gxPipeline->setUniform1i(
        "enableDither",
        static_cast<int>(bp.raster.dither && bp.raster.pixelFormat == 1));
    gxPipeline->setUniform1i("efbPixelFormat", bp.raster.pixelFormat);

    gxPipeline->setUniform1i("zTextureOp", static_cast<int>(bp.zTexture.op));
    gxPipeline->setUniform1i("zTextureFormat",
                             static_cast<int>(bp.zTexture.format));
    gxPipeline->setUniform1i("zTextureBias",
                             static_cast<int>(bp.zTexture.bias));

    gxPipeline->setUniform1i("fogType", static_cast<int>(bp.fog.type));
    gxPipeline->setUniform1f("fogA", bp.fog.a);
    gxPipeline->setUniform1i("fogBMagnitude",
                             static_cast<int>(bp.fog.bMagnitude));
    gxPipeline->setUniform1i("fogBShift", static_cast<int>(bp.fog.bShift));
    gxPipeline->setUniform1f("fogC", bp.fog.c);

    gxPipeline->setUniform3f("fogColor", bp.fog.color.r, bp.fog.color.g,
                             bp.fog.color.b);
    gxPipeline->setUniform1i("fogRangeEnabled",
                             static_cast<int>(bp.fog.rangeAdjustmentEnabled));
    const float viewportHalfWidth = std::max(
        std::abs(Device::globalDevice->gx.state.xf.viewport.xScale), 0.5f);
    gxPipeline->setUniform1f("fogRangeCenter",
                             (static_cast<float>(bp.fog.rangeCenter) - 342.0f) /
                                     viewportHalfWidth -
                                 1.0f);
    gxPipeline->setUniform1f("fogViewportWidth", viewportHalfWidth * 2.0f);
    for (size_t i = 0; i < bp.fog.rangeK.size(); ++i) {
        gxPipeline->setUniform1f("fogRangeK[" + std::to_string(i) + "]",
                                 bp.fog.rangeK[i] / 64.0f);
    }

    for (uint32_t i = 0; i < 4; ++i) {
        const auto &r = bp.tevRegisters[i];

        const std::string name = "tevRegister" + std::to_string(i);

        gxPipeline->setUniform4f(
            name, gx::tevComponentToFloat(r.r), gx::tevComponentToFloat(r.g),
            gx::tevComponentToFloat(r.b), gx::tevComponentToFloat(r.a));
    }

    for (uint32_t i = 0; i < 4; ++i) {
        const auto &k = bp.konstRegisters[i];

        const std::string name = "konst" + std::to_string(i);

        gxPipeline->setUniform4f(
            name, gx::tevComponentToFloat(k.r), gx::tevComponentToFloat(k.g),
            gx::tevComponentToFloat(k.b), gx::tevComponentToFloat(k.a));
    }

    for (uint32_t i = 0; i < 4; ++i) {
        const auto &s = bp.tevSwapTables[i];

        const std::string name = "swapTable" + std::to_string(i);

        gxPipeline->setUniform4f(name, s.r, s.g, s.b, s.a);
    }

    for (int i = 0; i < bp.tevStageCount; ++i) {
        applyTevState(gxPipeline, i);
    }

    for (uint32_t i = 0; i < bp.indirectStages.size(); ++i) {
        const auto &stage = bp.indirectStages[i];
        const std::string name = "indirectStages[" + std::to_string(i) + "]";
        gxPipeline->setUniform1i(name + ".texCoord", stage.texCoord);
        gxPipeline->setUniform1i(name + ".texMap", stage.texMap);
        gxPipeline->setUniform1i(name + ".scaleS", stage.scaleS);
        gxPipeline->setUniform1i(name + ".scaleT", stage.scaleT);
    }

    for (uint32_t i = 0; i < bp.indirectMatrices.size(); ++i) {
        const auto &matrix = bp.indirectMatrices[i];
        const std::string prefix = "indirectMatrix" + std::to_string(i);
        gxPipeline->setUniform4f(prefix + "A", matrix.m[0][0], matrix.m[0][1],
                                 matrix.m[0][2], matrix.exponent);
        gxPipeline->setUniform4f(prefix + "B", matrix.m[1][0], matrix.m[1][1],
                                 matrix.m[1][2], matrix.exponent);
    }

    static constexpr const char *textureNames[8] = {
        "tex0", "tex1", "tex2", "tex3", "tex4", "tex5", "tex6", "tex7"};

    for (uint32_t i = 0; i < 8; ++i) {
        const std::string sizeName = "textureSizes[" + std::to_string(i) + "]";
        const float width = boundTextures[i] ? boundTextures[i]->width : 1.0f;
        const float height = boundTextures[i] ? boundTextures[i]->height : 1.0f;
        gxPipeline->setUniform4f(sizeName, width, height, 1.0f / width,
                                 1.0f / height);

        if (!textureValid[i])
            continue;
        if (!boundTextures[i])
            continue;

        gxPipeline->bindTexture(textureNames[i], boundTextures[i], i);
    }

    commandBuffer->draw(static_cast<uint32_t>(vertices.size()));
    commandBuffer->resetScissor();
    commandBuffer->endPass();
    commandBuffer->commit();
    device->submitCommandBuffer(commandBuffer);
    vertices.clear();
}

void GXRenderer::ensureXFB(uint32_t width, uint32_t height) {
    width = std::clamp(width, 1u, EFB_WIDTH);
    height = std::clamp(height, 1u, XFB_MAX_HEIGHT);

    if (xfb.texture && xfb.width == width && xfb.height == height)
        return;

    xfb.width = width;
    xfb.height = height;
    xfb.texture =
        opal::Texture::create(opal::TextureType::Texture2D,
                              opal::TextureFormat::Rgba8, width, height);
    xfb.framebuffer = opal::Framebuffer::create(width, height);

    opal::Attachment attachment{};
    attachment.type = opal::Attachment::Type::Color;
    attachment.texture = xfb.texture;
    xfb.framebuffer->addAttachment(attachment);

    xfb.renderPass = opal::RenderPass::create();
    xfb.renderPass->setFramebuffer(xfb.framebuffer);
    xfb.valid = false;
}

void GXRenderer::copyEFBToXFB(const GXBPCopyState &copy) {
    const uint32_t sourceX = std::min<uint32_t>(copy.sourceX, EFB_WIDTH - 1);
    const uint32_t sourceY = std::min<uint32_t>(copy.sourceY, EFB_HEIGHT - 1);
    const uint32_t requestedWidth =
        copy.sourceWidth == 0 ? EFB_WIDTH : copy.sourceWidth;
    const uint32_t requestedHeight =
        copy.sourceHeight == 0 ? EFB_HEIGHT : copy.sourceHeight;
    const uint32_t width =
        std::min<uint32_t>(requestedWidth, EFB_WIDTH - sourceX);
    const uint32_t sourceHeight =
        std::min<uint32_t>(requestedHeight, EFB_HEIGHT - sourceY);
    const float yScale = copy.scaleInverted
                             ? 256.0f / std::max<uint16_t>(copy.yScale, 1)
                             : static_cast<float>(copy.yScale) / 256.0f;
    const uint32_t height = std::clamp<uint32_t>(
        static_cast<uint32_t>(1.0f + (sourceHeight - 1) * yScale), 1,
        XFB_MAX_HEIGHT);
    ensureXFB(width, height);

    xfb.address = copy.xfbAddress;
    xfb.stride = copy.xfbStride;

    if (sourceX == 0 && sourceY == 0 && width == EFB_WIDTH &&
        sourceHeight == EFB_HEIGHT && height == EFB_HEIGHT) {
        auto resolve = opal::ResolveAction::createForColorAttachment(
            efbFramebuffer, xfb.framebuffer, 0);
        auto commandBuffer = device->acquireCommandBuffer();
        commandBuffer->start();
        commandBuffer->performResolve(resolve);
        commandBuffer->commit();
        device->submitCommandBuffer(commandBuffer);
        xfb.valid = true;
    } else {
        presentPipeline->bindTexture("xfbTexture", efbColor, 0);

        const auto copyVertices = makeFullscreenVertices(
            static_cast<float>(sourceX) / EFB_WIDTH,
            static_cast<float>(sourceY) / EFB_HEIGHT,
            static_cast<float>(width) / EFB_WIDTH,
            static_cast<float>(sourceHeight) / EFB_HEIGHT);
        auto copyBuffer = opal::Buffer::create(
            opal::BufferUsage::VertexBuffer, sizeof(copyVertices),
            copyVertices.data(), opal::MemoryUsageType::CPUToGPU);
        auto copyDrawingState = opal::DrawingState::create(copyBuffer);

        auto commandBuffer = device->acquireCommandBuffer();
        commandBuffer->start();
        commandBuffer->beginPass(xfb.renderPass);
        commandBuffer->bindPipeline(presentPipeline);
        commandBuffer->bindDrawingState(copyDrawingState);
        commandBuffer->draw(6);
        commandBuffer->endPass();
        commandBuffer->commit();
        device->submitCommandBuffer(commandBuffer);
        xfb.valid = true;
    }
}

void GXRenderer::writeXFBToMemory(const GXBPCopyState &copy) {
    if (!xfb.valid || copy.xfbStride == 0 || xfb.width == 0 || xfb.height == 0)
        return;

    xfbRGBA.resize(static_cast<size_t>(xfb.width) * xfb.height * 4);
    xfb.texture->readData(xfbRGBA.data(), opal::TextureDataFormat::Rgba);

    const uint32_t rowBytes = ((xfb.width + 1) & ~1u) * 2;
    const size_t outputSize =
        static_cast<size_t>(xfb.height - 1) * copy.xfbStride + rowBytes;
    const ResolvedAddress first = Bus::resolveAddress(copy.xfbAddress);
    const ResolvedAddress last = Bus::resolveAddress(
        copy.xfbAddress + static_cast<uint32_t>(outputSize - 1));
    if (first.region == MemoryRegion::Invalid || first.region != last.region)
        return;
    xfbEncoded.assign(outputSize, 0);
    xfbScanout.assign(xfbRGBA.size(), 255);
    static const auto gammaTable = [] {
        std::array<std::array<uint8_t, 256>, 4> table{};
        constexpr float values[] = {1.0f, 1.7f, 2.2f, 2.2f};
        for (size_t gamma = 0; gamma < table.size(); ++gamma) {
            const float reciprocal = 1.0f / values[gamma];
            for (size_t value = 0; value < table[gamma].size(); ++value)
                table[gamma][value] = static_cast<uint8_t>(std::lround(
                    std::pow(value / 255.0f, reciprocal) * 255.0f));
        }
        return table;
    }();
    const auto &corrected = gammaTable[copy.gamma & 3u];
    const auto clampByte = [](int value) {
        return static_cast<uint8_t>(std::clamp(value, 0, 255));
    };

    for (uint32_t y = 0; y < xfb.height; ++y) {
        const size_t sourceRow = static_cast<size_t>(y) * xfb.width * 4;
        const size_t destinationRow = static_cast<size_t>(y) * copy.xfbStride;
        for (uint32_t x = 0; x < xfb.width; x += 2) {
            const uint32_t secondX = std::min(x + 1, xfb.width - 1);
            const size_t first = sourceRow + static_cast<size_t>(x) * 4;
            const size_t second = sourceRow + static_cast<size_t>(secondX) * 4;
            const int r0 = corrected[xfbRGBA[first]];
            const int g0 = corrected[xfbRGBA[first + 1]];
            const int b0 = corrected[xfbRGBA[first + 2]];
            const int r1 = corrected[xfbRGBA[second]];
            const int g1 = corrected[xfbRGBA[second + 1]];
            const int b1 = corrected[xfbRGBA[second + 2]];
            const int y0 = (66 * r0 + 129 * g0 + 25 * b0 + 4096 + 128) >> 8;
            const int y1 = (66 * r1 + 129 * g1 + 25 * b1 + 4096 + 128) >> 8;
            const int r = (r0 + r1) / 2;
            const int g = (g0 + g1) / 2;
            const int b = (b0 + b1) / 2;
            const int u = (-38 * r - 74 * g + 112 * b + 32768 + 128) >> 8;
            const int v = (112 * r - 94 * g - 18 * b + 32768 + 128) >> 8;
            const size_t destination =
                destinationRow + static_cast<size_t>(x) * 2;
            xfbEncoded[destination] = clampByte(y0);
            xfbEncoded[destination + 1] = clampByte(u);
            xfbEncoded[destination + 2] = clampByte(y1);
            xfbEncoded[destination + 3] = clampByte(v);

            const auto decode = [&](uint32_t pixelX, int luminance) {
                const int c = luminance - 16;
                const int d = u - 128;
                const int e = v - 128;
                const size_t pixel =
                    (static_cast<size_t>(y) * xfb.width + pixelX) * 4;
                xfbScanout[pixel] =
                    clampByte((298 * c + 409 * e + 128) >> 8);
                xfbScanout[pixel + 1] =
                    clampByte((298 * c - 100 * d - 208 * e + 128) >> 8);
                xfbScanout[pixel + 2] =
                    clampByte((298 * c + 516 * d + 128) >> 8);
            };
            decode(x, y0);
            if (secondX != x)
                decode(secondX, y1);
        }
    }

    Bus::writeBlock(copy.xfbAddress, xfbEncoded);
    xfb.texture->updateData(xfbScanout.data(), static_cast<int>(xfb.width),
                            static_cast<int>(xfb.height),
                            opal::TextureDataFormat::Rgba);
}

void GXRenderer::clearEFB(const GXColor &color, uint32_t depth) {
    auto commandBuffer = device->acquireCommandBuffer();
    commandBuffer->start();
    commandBuffer->beginPass(efbRenderPass);
    commandBuffer->clear(color.r, color.g, color.b, color.a,
                         static_cast<float>(depth & 0xFFFFFF) / 16777215.0f);
    commandBuffer->bindPipeline(gxPipeline);
    commandBuffer->draw(0);
    commandBuffer->endPass();
    commandBuffer->commit();
    device->submitCommandBuffer(commandBuffer);
}

void GXRenderer::readXFBFromMemory() {
    if (!xfb.valid || xfb.stride == 0 || xfb.width == 0 || xfb.height == 0)
        return;

    const uint32_t rowBytes = ((xfb.width + 1) & ~1u) * 2;
    const size_t inputSize =
        static_cast<size_t>(xfb.height - 1) * xfb.stride + rowBytes;
    const ResolvedAddress first = Bus::resolveAddress(xfb.address);
    const ResolvedAddress last =
        Bus::resolveAddress(xfb.address + static_cast<uint32_t>(inputSize - 1));
    if (first.region == MemoryRegion::Invalid || first.region != last.region)
        return;

    std::vector<uint8_t> encoded(inputSize);
    std::vector<uint8_t> rgba(static_cast<size_t>(xfb.width) * xfb.height * 4,
                              255);
    Bus::readBlock(xfb.address, encoded);

    const auto clampByte = [](int value) {
        return static_cast<uint8_t>(std::clamp(value, 0, 255));
    };

    for (uint32_t y = 0; y < xfb.height; ++y) {
        const size_t sourceRow = static_cast<size_t>(y) * xfb.stride;
        for (uint32_t x = 0; x < xfb.width; x += 2) {
            const size_t source = sourceRow + static_cast<size_t>(x) * 2;
            const int y0 = encoded[source];
            const int u = encoded[source + 1];
            const int y1 = encoded[source + 2];
            const int v = encoded[source + 3];

            const auto decode = [&](uint32_t pixelX, int luminance) {
                const int c = luminance - 16;
                const int d = u - 128;
                const int e = v - 128;
                const size_t pixel =
                    (static_cast<size_t>(y) * xfb.width + pixelX) * 4;
                rgba[pixel] = clampByte((298 * c + 409 * e + 128) >> 8);
                rgba[pixel + 1] =
                    clampByte((298 * c - 100 * d - 208 * e + 128) >> 8);
                rgba[pixel + 2] = clampByte((298 * c + 516 * d + 128) >> 8);
            };

            decode(x, y0);
            if (x + 1 < xfb.width)
                decode(x + 1, y1);
        }
    }

    xfb.texture->updateData(rgba.data(), static_cast<int>(xfb.width),
                            static_cast<int>(xfb.height),
                            opal::TextureDataFormat::Rgba);
}

void GXRenderer::presentXFB() {
    flushEFB();

    if (!xfb.valid) {
        GXBPCopyState copy{};
        copy.sourceWidth = EFB_WIDTH;
        copy.sourceHeight = EFB_HEIGHT;
        copyEFBToXFB(copy);
    }

    presentPipeline->bindTexture("xfbTexture", xfb.texture, 0);

    auto commandBuffer = device->acquireCommandBuffer();
    commandBuffer->start();
    commandBuffer->beginPass(displayRenderPass);
    commandBuffer->clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    commandBuffer->bindPipeline(presentPipeline);
    commandBuffer->bindDrawingState(fullscreenDrawingState);
    commandBuffer->draw(6);
    commandBuffer->endPass();
    commandBuffer->commit();
    device->submitCommandBuffer(commandBuffer);
}

void GXRenderer::setRasterState(const GXRasterState &state) {
    currentRasterState = state;
    rasterStateDirty = true;
}

void GXRenderer::rebuildGXPipeline() {
    gxPipeline = opal::Pipeline::create();

    gxPipeline->setShaderProgram(gxShaderProgram);
    gxPipeline->setPrimitiveStyle(opal::PrimitiveStyle::Triangles);
    gxPipeline->setRasterizerMode(opal::RasterizerMode::Fill);
    gxPipeline->setCullMode(currentRasterState.cullMode);
    gxPipeline->enableDepthTest(currentRasterState.depthTest);
    gxPipeline->setDepthCompareOp(currentRasterState.depthCompare);
    gxPipeline->enableDepthWrite(currentRasterState.depthWrite);
    gxPipeline->enableBlending(currentRasterState.blendEnabled);
    gxPipeline->setBlendFunc(currentRasterState.subtractBlend
                                 ? opal::BlendFunc::One
                                 : currentRasterState.srcBlend,
                             currentRasterState.subtractBlend
                                 ? opal::BlendFunc::One
                                 : currentRasterState.dstBlend);
    gxPipeline->setBlendEquation(currentRasterState.subtractBlend
                                     ? opal::BlendEquation::ReverseSubtract
                                     : opal::BlendEquation::Add);
    gxPipeline->enableLogicOp(currentRasterState.logicOpEnabled &&
                              !currentRasterState.blendEnabled);
    gxPipeline->setLogicOp(decodeLogicOp(currentRasterState.logicOp));

    gxPipeline->setColorWriteMask(
        currentRasterState.colorWrite, currentRasterState.colorWrite,
        currentRasterState.colorWrite, currentRasterState.alphaWrite);

    gxPipeline->setVertexAttributes(gxAttributes, gxBinding);
    gxPipeline->setFrontFace(currentRasterState.frontFace);

    gxPipeline->build();

    rasterStateDirty = false;
}

void GXRenderer::setAlphaTestState(const GXAlphaTestState &state) {
    currentAlphaTest = state;
}

void GXRenderer::setTexture(uint32_t unit, const GXDecodedTexture &texture,
                            const GXTextureState &state) {
    if (unit >= 8) {
        return;
    }

    if (texture.rgba.empty() || texture.width == 0 || texture.height == 0) {
        boundTextures[unit] = nullptr;
        textureValid[unit] = false;
        return;
    }

    if (boundTextures[unit] && boundTextures[unit]->width == texture.width &&
        boundTextures[unit]->height == texture.height) {
        boundTextures[unit]->updateData(texture.rgba.data(), texture.width,
                                        texture.height,
                                        opal::TextureDataFormat::Rgba);
    } else {
        boundTextures[unit] = opal::Texture::create(
            opal::TextureType::Texture2D, opal::TextureFormat::Rgba8,
            texture.width, texture.height, opal::TextureDataFormat::Rgba,
            texture.rgba.data());
    }

    auto &tex = boundTextures[unit];
    textureValid[unit] = true;

    tex->setParameters(decodeWrapMode(state.wrapS), decodeWrapMode(state.wrapT),
                       decodeMinFilter(state.minFilter),
                       decodeMagFilter(state.magFilter));
}

void GXRenderer::applyTevState(std::shared_ptr<opal::Pipeline> &pipeline,
                               uint8_t stage) {
    const auto &tevStage = Device::globalDevice->gx.state.bp.tevStages[stage];

    const std::string baseName = "tevStages[" + std::to_string(stage) + "]";

    pipeline->setUniform1i(baseName + ".texCoord", tevStage.order.texCoord);
    pipeline->setUniform1i(baseName + ".texMap", tevStage.order.texMap);
    pipeline->setUniform1i(baseName + ".colorChannel",
                           tevStage.order.colorChannel);
    pipeline->setUniform1i(baseName + ".textureEnabled",
                           tevStage.order.textureEnabled ? 1 : 0);

    pipeline->setUniform1i(baseName + ".colorA",
                           static_cast<int>(tevStage.color.a));
    pipeline->setUniform1i(baseName + ".colorB",
                           static_cast<int>(tevStage.color.b));
    pipeline->setUniform1i(baseName + ".colorC",
                           static_cast<int>(tevStage.color.c));
    pipeline->setUniform1i(baseName + ".colorD",
                           static_cast<int>(tevStage.color.d));

    pipeline->setUniform1i(baseName + ".colorBias",
                           static_cast<int>(tevStage.color.bias));
    pipeline->setUniform1i(baseName + ".colorOp",
                           static_cast<int>(tevStage.color.op));
    pipeline->setUniform1i(baseName + ".colorClamp",
                           static_cast<int>(tevStage.color.clamp));
    pipeline->setUniform1i(baseName + ".colorScale",
                           static_cast<int>(tevStage.color.scale));
    pipeline->setUniform1i(baseName + ".colorOutput",
                           static_cast<int>(tevStage.color.output));

    pipeline->setUniform1i(baseName + ".alphaA",
                           static_cast<int>(tevStage.alpha.a));
    pipeline->setUniform1i(baseName + ".alphaB",
                           static_cast<int>(tevStage.alpha.b));
    pipeline->setUniform1i(baseName + ".alphaC",
                           static_cast<int>(tevStage.alpha.c));
    pipeline->setUniform1i(baseName + ".alphaD",
                           static_cast<int>(tevStage.alpha.d));

    pipeline->setUniform1i(baseName + ".alphaBias",
                           static_cast<int>(tevStage.alpha.bias));
    pipeline->setUniform1i(baseName + ".alphaOp",
                           static_cast<int>(tevStage.alpha.op));
    pipeline->setUniform1i(baseName + ".alphaClamp",
                           static_cast<int>(tevStage.alpha.clamp));
    pipeline->setUniform1i(baseName + ".alphaScale",
                           static_cast<int>(tevStage.alpha.scale));
    pipeline->setUniform1i(baseName + ".alphaOutput",
                           static_cast<int>(tevStage.alpha.output));

    pipeline->setUniform1i(baseName + ".rasterSwap",
                           static_cast<int>(tevStage.alpha.rasterSwap));
    pipeline->setUniform1i(baseName + ".textureSwap",
                           static_cast<int>(tevStage.alpha.textureSwap));
    pipeline->setUniform1i(baseName + ".konstColorSel",
                           static_cast<int>(tevStage.konstColorSel));
    pipeline->setUniform1i(baseName + ".konstAlphaSel",
                           static_cast<int>(tevStage.konstAlphaSel));

    const auto &indirect = Device::globalDevice->gx.state.bp.tevIndirect[stage];
    pipeline->setUniform1i(baseName + ".indirectStage", indirect.stage);
    pipeline->setUniform1i(baseName + ".indirectFormat", indirect.format);
    pipeline->setUniform1i(baseName + ".indirectBias", indirect.bias);
    pipeline->setUniform1i(baseName + ".indirectAlpha", indirect.alphaSelect);
    pipeline->setUniform1i(baseName + ".indirectMatrix", indirect.matrix);
    pipeline->setUniform1i(baseName + ".indirectWrapS", indirect.wrapS);
    pipeline->setUniform1i(baseName + ".indirectWrapT", indirect.wrapT);
    pipeline->setUniform1i(baseName + ".indirectUseOriginalLod",
                           indirect.useOriginalLod ? 1 : 0);
    pipeline->setUniform1i(baseName + ".indirectAddPrevious",
                           indirect.addPrevious ? 1 : 0);
}
