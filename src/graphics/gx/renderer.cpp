
#include "SDL3/SDL_video.h"
#include "graphics/gx.h"
#include "graphics/shader.h"
#include "opal/opal.h"
#include <cstddef>
#include <memory>

void GXRenderer::drawTriangle(const GXRenderVertex &a, const GXRenderVertex &b,
                              const GXRenderVertex &c) {
    pendingVertices.push_back(a);
    pendingVertices.push_back(b);
    pendingVertices.push_back(c);
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

    framebuffer = device->getDefaultFramebuffer();

    renderPass = opal::RenderPass::create();
    renderPass->setFramebuffer(framebuffer);

    auto globalShader =
        opal::Shader::createFromSource(SHADER, opal::ShaderType::Vertex);

    auto vertexShader =
        globalShader->forFunction("vertexMain", opal::ShaderType::Vertex);
    auto fragmentShader =
        globalShader->forFunction("fragmentMain", opal::ShaderType::Fragment);

    vertexShader->compile();
    fragmentShader->compile();

    shaderProgram = opal::ShaderProgram::create();
    shaderProgram->attachShader(vertexShader);
    shaderProgram->attachShader(fragmentShader);
    shaderProgram->link();

    pipeline = opal::Pipeline::create();

    pipeline->setShaderProgram(shaderProgram);
    pipeline->setPrimitiveStyle(opal::PrimitiveStyle::Triangles);
    pipeline->setRasterizerMode(opal::RasterizerMode::Fill);
    pipeline->setCullMode(opal::CullMode::None);

    pipeline->enableDepthTest(false);
    pipeline->enableDepthWrite(false);

    pipeline->enableBlending(false);

    std::vector<opal::VertexAttribute> attributes = {
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

    opal::VertexBinding binding{};
    binding.stride = sizeof(GXRenderVertex);
    binding.inputRate = opal::VertexBindingInputRate::Vertex;

    pipeline->setVertexAttributes(attributes, binding);

    pipeline->build();
}

void GXRenderer::finishGXBatch() {
    if (pendingVertices.empty())
        return;

    displayVertices = pendingVertices;
    pendingVertices.clear();
}

void GXRenderer::uploadVertices() {
    if (displayVertices.empty())
        return;

    const size_t size = displayVertices.size() * sizeof(GXRenderVertex);

    vertexBuffer = opal::Buffer::create(opal::BufferUsage::VertexBuffer, size,
                                        displayVertices.data(),
                                        opal::MemoryUsageType::CPUToGPU);

    drawingState = opal::DrawingState::create(vertexBuffer);
}

void GXRenderer::present() {
    if (displayVertices.empty())
        return;

    uploadVertices();

    auto commandBuffer = device->acquireCommandBuffer();

    commandBuffer->start();
    commandBuffer->beginPass(renderPass);
    commandBuffer->clear(0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    commandBuffer->bindPipeline(pipeline);
    commandBuffer->bindDrawingState(drawingState);
    commandBuffer->draw(static_cast<uint32_t>(displayVertices.size()));
    commandBuffer->endPass();
    commandBuffer->commit();

    device->submitCommandBuffer(commandBuffer);
}