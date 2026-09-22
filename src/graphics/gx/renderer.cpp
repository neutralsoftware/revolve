#include "SDL3/SDL_video.h"
#include "graphics/gx.h"
#include "graphics/shader.h"
#include "opal/opal.h"
#include <algorithm>
#include <cstddef>
#include <memory>

namespace {
struct FullscreenVertex {
    float x;
    float y;
    float u;
    float v;
};

constexpr FullscreenVertex FULLSCREEN_VERTICES[] = {
    {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, -1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {-1.0f, 1.0f, 0.0f, 0.0f},
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
}

void GXRenderer::drawTriangle(const GXRenderVertex &a, const GXRenderVertex &b,
                              const GXRenderVertex &c) {
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
}

void GXRenderer::initialize() {
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

    auto gxShader = opal::Shader::createFromSource(
        MINIMAL_SHADER, opal::ShaderType::Vertex);
    auto gxVertexShader =
        gxShader->forFunction("vertexMain", opal::ShaderType::Vertex);
    auto gxFragmentShader =
        gxShader->forFunction("fragmentMain", opal::ShaderType::Fragment);
    gxVertexShader->compile();
    gxFragmentShader->compile();

    gxShaderProgram = opal::ShaderProgram::create();
    gxShaderProgram->attachShader(gxVertexShader);
    gxShaderProgram->attachShader(gxFragmentShader);
    gxShaderProgram->link();

    gxPipeline = opal::Pipeline::create();
    gxPipeline->setShaderProgram(gxShaderProgram);
    gxPipeline->setPrimitiveStyle(opal::PrimitiveStyle::Triangles);
    gxPipeline->setRasterizerMode(opal::RasterizerMode::Fill);
    gxPipeline->setCullMode(opal::CullMode::None);
    gxPipeline->enableDepthTest(false);
    gxPipeline->enableDepthWrite(false);
    gxPipeline->enableBlending(false);

    std::vector<opal::VertexAttribute> gxAttributes = {
        {.name = "inPosition",
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

    opal::VertexBinding gxBinding{};
    gxBinding.stride = sizeof(GXRenderVertex);
    gxBinding.inputRate = opal::VertexBindingInputRate::Vertex;
    gxPipeline->setVertexAttributes(gxAttributes, gxBinding);
    gxPipeline->build();

    clearEFB({}, 0xFFFFFF);
    createPresentPipeline();
}

void GXRenderer::createEFB() {
    efbColor = opal::Texture::create(opal::TextureType::Texture2D,
                                     opal::TextureFormat::Rgba8, EFB_WIDTH,
                                     EFB_HEIGHT);
    efbDepth = opal::Texture::create(
        opal::TextureType::Texture2D,
        opal::TextureFormat::Depth24Stencil8, EFB_WIDTH, EFB_HEIGHT,
        opal::TextureDataFormat::DepthComponent);
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
    auto shader = opal::Shader::createFromSource(
        FULLSCREEN_SHADER, opal::ShaderType::Vertex);
    auto vertexShader =
        shader->forFunction("vertexMain", opal::ShaderType::Vertex);
    auto fragmentShader =
        shader->forFunction("fragmentMain", opal::ShaderType::Fragment);
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

    const size_t size = vertices.size() * sizeof(GXRenderVertex);
    auto vertexBuffer =
        opal::Buffer::create(opal::BufferUsage::VertexBuffer, size,
                             vertices.data(), opal::MemoryUsageType::CPUToGPU);
    auto drawingState = opal::DrawingState::create(vertexBuffer);
    auto commandBuffer = device->acquireCommandBuffer();
    commandBuffer->start();
    commandBuffer->beginPass(efbRenderPass);
    commandBuffer->bindPipeline(gxPipeline);
    commandBuffer->bindDrawingState(drawingState);
    commandBuffer->draw(static_cast<uint32_t>(vertices.size()));
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

    const auto copyVertices = makeFullscreenVertices(
        static_cast<float>(sourceX) / EFB_WIDTH,
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
