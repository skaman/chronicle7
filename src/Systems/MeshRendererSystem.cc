#include "MeshRendererSystem.h"

namespace chronicle {

using namespace entt::literals;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

const std::vector<Vertex> Vertices = { { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },

    { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
    { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } } };

const std::vector<uint16_t> Indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };

MeshRendererSystem::MeshRendererSystem()
{
    CHRZONE_RENDERER_SYSTEM;

    std::string filename = "D:\\Progetti\\glTF-Sample-Models\\2.0\\Triangle\\glTF-Embedded\\Triangle.gltf";
    // std::string filename = "D:\\Progetti\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\Sponza.gltf";
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    auto test = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    // https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/base/VulkanglTFModel.cpp

    const auto& renderer = entt::locator<Renderer>::value();

    // render pass
    RenderPassInfo renderPassInfo = {};
    renderPassInfo.colorAttachmentFormat = renderer.swapChainFormat();
    renderPassInfo.images = renderer.swapChainImages();
    _renderPass = renderer.createRenderPass(renderPassInfo);

    // texture
    ImageInfo imageInfo = {};
    imageInfo.filename = "D:\\texture.jpg";
    _texture = renderer.createImage(imageInfo);

    // descriptor sets
    _descriptorSets.reserve(MAX_FRAMES_IN_FLIGHT);
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto descriptorSet = renderer.createDescriptorSet();
        descriptorSet->addUniform<UniformBufferObject>("ubo"_hs, ShaderStage::Vertex);
        descriptorSet->addSampler(ShaderStage::Fragment, _texture);
        descriptorSet->build();
        _descriptorSets.push_back(descriptorSet);
    }

    // pipeline
    PipelineInfo pipelineInfo = {};
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.shaders[ShaderStage::Vertex] = "Shaders/triangle.vert.bin";
    pipelineInfo.shaders[ShaderStage::Fragment] = "Shaders/triangle.frag.bin";

    VertexBufferInfo vertexBufferInfo = {};
    vertexBufferInfo.stride = sizeof(Vertex);

    AttributeDescriptionInfo descriptorInfo0 = {};
    descriptorInfo0.format = Format::R32G32B32Sfloat;
    descriptorInfo0.offset = offsetof(Vertex, pos);
    vertexBufferInfo.attributeDescriptions.push_back(descriptorInfo0);

    AttributeDescriptionInfo descriptorInfo1 = {};
    descriptorInfo1.format = Format::R32G32B32Sfloat;
    descriptorInfo1.offset = offsetof(Vertex, color);
    vertexBufferInfo.attributeDescriptions.push_back(descriptorInfo1);

    AttributeDescriptionInfo descriptorInfo2 = {};
    descriptorInfo2.format = Format::R32G32Sfloat;
    descriptorInfo2.offset = offsetof(Vertex, texCoord);
    vertexBufferInfo.attributeDescriptions.push_back(descriptorInfo2);

    pipelineInfo.vertexBuffers.push_back(vertexBufferInfo);

    _pipeline = renderer.createPipeline(pipelineInfo);

    // vertex and index buffer
    _vertexBuffer = renderer.createVertexBuffer();
    _indexBuffer = renderer.createIndexBuffer();

    _vertexBuffer->set((void*)Vertices.data(), sizeof(Vertices[0]) * Vertices.size());
    _indexBuffer->set((void*)Indices.data(), sizeof(Indices[0]) * Indices.size());

    // command buffer
    _commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto commandBuffer = renderer.createCommandBuffer();
        _commandBuffers.push_back(commandBuffer);
    }

    // sync objects
    _imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _imageAvailableSemaphores.push_back(renderer.createSemaphore());
        _renderFinishedSemaphores.push_back(renderer.createSemaphore());
        _inFlightFences.push_back(renderer.createFence());
    }
}

MeshRendererSystem::~MeshRendererSystem()
{
    CHRZONE_RENDERER_SYSTEM

    _texture.reset();
    _inFlightFences.clear();
    _renderFinishedSemaphores.clear();
    _imageAvailableSemaphores.clear();
    _commandBuffers.clear();
    _indexBuffer.reset();
    _vertexBuffer.reset();
    _pipeline.reset();
    _descriptorSets.clear();
    _renderPass.reset();
}

void MeshRendererSystem::run(entt::registry& registry)
{
    CHRZONE_RENDERER_SYSTEM

    auto& renderer = entt::locator<Renderer>::value();

    renderer.waitForFence(_inFlightFences[_currentFrame]);

    auto imageIndex = renderer.acquireNextImage(_imageAvailableSemaphores[_currentFrame]);

    renderer.resetFence(_inFlightFences[_currentFrame]);
    _commandBuffers[_currentFrame]->reset();

    recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);
    updateUniformBuffer(_currentFrame);

    renderer.submit(_inFlightFences[_currentFrame], _imageAvailableSemaphores[_currentFrame],
        _renderFinishedSemaphores[_currentFrame], _commandBuffers[_currentFrame]);

    renderer.present(_renderFinishedSemaphores[_currentFrame], imageIndex);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    FrameMark;
}

void MeshRendererSystem::recordCommandBuffer(const std::shared_ptr<CommandBuffer>& commandBuffer, uint32_t imageIndex)
{
    CHRZONE_RENDERER_SYSTEM

    const auto& renderer = entt::locator<Renderer>::value();

    auto extent = renderer.swapChainExtent();

    commandBuffer->begin();
    commandBuffer->beginRenderPass(_renderPass, RectInt2D({ 0, 0 }, extent), imageIndex);

    commandBuffer->setViewport(
        RectFloat2D({ 0.0f, 0.0f }, { static_cast<float>(extent.width), static_cast<float>(extent.height) }), 0.0f,
        1.0f);
    commandBuffer->setScissor(RectInt2D({ 0, 0 }, extent));

    commandBuffer->bindPipeline(_pipeline);
    commandBuffer->bindVertexBuffer(_vertexBuffer);
    commandBuffer->bindIndexBuffer(_indexBuffer);
    commandBuffer->bindDescriptorSet(_descriptorSets[_currentFrame], 0);

    commandBuffer->drawIndexed(static_cast<uint32_t>(Indices.size()), 1);

    commandBuffer->endRenderPass();
    commandBuffer->end();
}

void MeshRendererSystem::updateUniformBuffer(uint32_t currentFrame)
{
    CHRZONE_RENDERER_SYSTEM

    const auto& renderer = entt::locator<Renderer>::value();

    auto extent = renderer.swapChainExtent();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
        glm::radians(45.0f), static_cast<float>(extent.width) / static_cast<float>(extent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    _descriptorSets[currentFrame]->setUniform<UniformBufferObject>("ubo"_hs, ubo);
}

} // namespace chronicle