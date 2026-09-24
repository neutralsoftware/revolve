#include "SDL3/SDL_video.h"
#include "device.h"
#include "graphics/gx.h"
#include "graphics/shader.h"
#include "opal/opal.h"
#include <algorithm>
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
                    {.name = "inColor",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, r),
                     .location = 1,
                     .normalized = false,
                     .size = 4,
                     .stride = sizeof(GXRenderVertex)},
                    {.name = "inUV",
                     .type = opal::VertexAttributeType::Float,
                     .offset = offsetof(GXRenderVertex, u),
                     .location = 2,
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

    for (int i = 0; i < bp.tevStageCount; ++i) {
        applyTevState(gxPipeline, i);
    }

    static constexpr const char *textureNames[8] = {
        "tex0", "tex1", "tex2", "tex3", "tex4", "tex5", "tex6", "tex7"};

    for (uint32_t i = 0; i < 8; ++i) {
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
    height = std::clamp(height, 1u, EFB_HEIGHT);

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
    const uint32_t height =
        std::min<uint32_t>(requestedHeight, EFB_HEIGHT - sourceY);
    ensureXFB(width, height);

    xfb.address = copy.xfbAddress;
    xfb.stride = copy.xfbStride;

    if (sourceX == 0 && sourceY == 0 && width == EFB_WIDTH &&
        height == EFB_HEIGHT) {
        auto resolve = opal::ResolveAction::createForColorAttachment(
            efbFramebuffer, xfb.framebuffer, 0);
        auto commandBuffer = device->acquireCommandBuffer();
        commandBuffer->start();
        commandBuffer->performResolve(resolve);
        commandBuffer->commit();
        device->submitCommandBuffer(commandBuffer);
        xfb.valid = true;
        return;
    }

    presentPipeline->bindTexture("xfbTexture", efbColor, 0);

    const auto copyVertices =
        makeFullscreenVertices(static_cast<float>(sourceX) / EFB_WIDTH,
                               static_cast<float>(sourceY) / EFB_HEIGHT,
                               static_cast<float>(width) / EFB_WIDTH,
                               static_cast<float>(height) / EFB_HEIGHT);
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
    gxPipeline->setBlendFunc(currentRasterState.srcBlend,
                             currentRasterState.dstBlend);
    gxPipeline->setBlendEquation(currentRasterState.subtractBlend
                                     ? opal::BlendEquation::Subtract
                                     : opal::BlendEquation::Add);

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

    pipeline->setUniform1i(baseName + ".texCoord", tevStage.order.texMap);
    pipeline->setUniform1i(baseName + "texMap", tevStage.order.texCoord);
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
}