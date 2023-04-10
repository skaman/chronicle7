#include "MeshRenderSystem.h"

#include "Locator.h"
#include "Renderer/Renderer.h"

namespace chronicle {

using namespace entt::literals;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

MeshRenderSystem::MeshRenderSystem()
{
    CHRZONE_RENDERER_SYSTEM

    const auto renderer = Locator::renderer.get();

    _mesh = MeshAsset::load("D:\\viking_room.obj");
    _texture = TextureAsset::load("D:\\viking_room.png");

    // render pass
    RenderPassInfo renderPassInfo = {};
    renderPassInfo.colorAttachmentFormat = renderer->swapChainFormat();
    renderPassInfo.depthAttachmentFormat = renderer->depthFormat();
    renderPassInfo.images = renderer->swapChainImages();
    renderPassInfo.depthImage = renderer->depthImage();
    _renderPass = RenderPass::create(renderPassInfo);

    // command buffer
    _commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto commandBuffer = CommandBuffer::create();
        _commandBuffers.push_back(commandBuffer);
    }

    // sync objects
    _imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _imageAvailableSemaphores.push_back(Semaphore::create());
        _renderFinishedSemaphores.push_back(Semaphore::create());
        _inFlightFences.push_back(Fence::create());
    }

    // descriptor sets
    _descriptorSets.reserve(MAX_FRAMES_IN_FLIGHT);
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto descriptorSet = DescriptorSet::create();
        descriptorSet->addUniform<UniformBufferObject>("ubo"_hs, ShaderStage::Vertex);
        descriptorSet->addSampler(ShaderStage::Fragment, _texture->image());
        descriptorSet->build();
        _descriptorSets.push_back(descriptorSet);
    }

    // pipeline
    PipelineInfo pipelineInfo = {};
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.shaders[ShaderStage::Vertex] = "Shaders/triangle.vert.bin";
    pipelineInfo.shaders[ShaderStage::Fragment] = "Shaders/triangle.frag.bin";
    pipelineInfo.vertexBuffers.push_back(_mesh->bufferInfo());

    _pipeline = Pipeline::create(pipelineInfo);
}

MeshRenderSystem::~MeshRenderSystem()
{
    CHRZONE_RENDERER_SYSTEM

    _inFlightFences.clear();
    _renderFinishedSemaphores.clear();
    _imageAvailableSemaphores.clear();
    _commandBuffers.clear();
    _mesh.reset();
    _texture.reset();
    _pipeline.reset();
    _descriptorSets.clear();
    _renderPass.reset();
}

void MeshRenderSystem::activate() { }

void MeshRenderSystem::deactivate() { }

void MeshRenderSystem::run(entt::registry& registry)
{
    CHRZONE_RENDERER_SYSTEM

    const auto renderer = Locator::renderer.get();

    renderer->waitForFence(_inFlightFences[_currentFrame]);

    auto imageIndex = renderer->acquireNextImage(_imageAvailableSemaphores[_currentFrame]);

    renderer->resetFence(_inFlightFences[_currentFrame]);
    _commandBuffers[_currentFrame]->reset();

    recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);
    updateUniformBuffer(_currentFrame);

    renderer->submit(_inFlightFences[_currentFrame], _imageAvailableSemaphores[_currentFrame],
        _renderFinishedSemaphores[_currentFrame], _commandBuffers[_currentFrame]);

    renderer->present(_renderFinishedSemaphores[_currentFrame], imageIndex);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    FrameMark;
}

void MeshRenderSystem::recordCommandBuffer(const CommandBufferRef& commandBuffer, uint32_t imageIndex)
{
    CHRZONE_RENDERER_SYSTEM

    const auto renderer = Locator::renderer.get();

    auto extent = renderer->swapChainExtent();

    commandBuffer->begin();
    commandBuffer->beginRenderPass(_renderPass, RectInt2D({ 0, 0 }, extent), imageIndex);

    commandBuffer->setViewport(
        RectFloat2D({ 0.0f, 0.0f }, { static_cast<float>(extent.width), static_cast<float>(extent.height) }), 0.0f,
        1.0f);
    commandBuffer->setScissor(RectInt2D({ 0, 0 }, extent));

    commandBuffer->bindPipeline(_pipeline);
    commandBuffer->bindVertexBuffer(_mesh->vertexBuffer(0));
    commandBuffer->bindIndexBuffer(_mesh->indexBuffer(0));
    commandBuffer->bindDescriptorSet(_descriptorSets[_currentFrame], 0);

    commandBuffer->drawIndexed(_mesh->indicesCount(0), 1);

    commandBuffer->endRenderPass();
    commandBuffer->end();
}

void MeshRenderSystem::updateUniformBuffer(uint32_t currentFrame)
{
    CHRZONE_RENDERER_SYSTEM

    const auto renderer = Locator::renderer.get();

    auto extent = renderer->swapChainExtent();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    float time = 0;

    UniformBufferObject ubo {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
        glm::radians(45.0f), static_cast<float>(extent.width) / static_cast<float>(extent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    _descriptorSets[currentFrame]->setUniform<UniformBufferObject>("ubo"_hs, ubo);
}

} // namespace chronicle