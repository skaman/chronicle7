// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "pch.h"

#include <Assets/MeshAsset.h>
#include <Assets/TextureAsset.h>
#include <Gui/Gui.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>

using namespace entt::literals;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class ExampleApp {
public:
    void init()
    {
        chronicle::Platform::init();
        chronicle::Renderer::init();

        CHRLOG_INFO("Start");

        // assets
        _mesh = chronicle::MeshAsset::load("D:\\viking_room.obj");
        _texture = chronicle::TextureAsset::load("D:\\viking_room.png");

        // render pass
        chronicle::RenderPassInfo renderPassInfo = {};
        renderPassInfo.colorAttachmentFormat = chronicle::Renderer::swapChainFormat();
        renderPassInfo.depthAttachmentFormat = chronicle::Renderer::depthFormat();
        renderPassInfo.images = chronicle::Renderer::swapChainImages();
        renderPassInfo.depthImage = chronicle::Renderer::depthImage();
        _renderPass = chronicle::RenderPass::create(renderPassInfo);

        chronicle::Gui::init(2, _renderPass);

        // command buffer
        _commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            auto commandBuffer = chronicle::CommandBuffer::create();
            _commandBuffers.push_back(commandBuffer);
        }

        // sync objects
        _imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        _renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        _inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            _imageAvailableSemaphores.push_back(chronicle::Semaphore::create());
            _renderFinishedSemaphores.push_back(chronicle::Semaphore::create());
            _inFlightFences.push_back(chronicle::Fence::create());
        }

        // descriptor sets
        _descriptorSets.reserve(MAX_FRAMES_IN_FLIGHT);
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            auto descriptorSet = chronicle::DescriptorSet::create();
            descriptorSet->addUniform<UniformBufferObject>("ubo"_hs, chronicle::ShaderStage::Vertex);
            descriptorSet->addSampler(chronicle::ShaderStage::Fragment, _texture->image());
            descriptorSet->build();
            _descriptorSets.push_back(descriptorSet);
        }

        // pipeline
        chronicle::PipelineInfo pipelineInfo = {};
        pipelineInfo.renderPass = _renderPass;
        pipelineInfo.shaders[chronicle::ShaderStage::Vertex] = "Shaders/triangle.vert.bin";
        pipelineInfo.shaders[chronicle::ShaderStage::Fragment] = "Shaders/triangle.frag.bin";
        pipelineInfo.vertexBuffers.push_back(_mesh->bufferInfo());

        _pipeline = chronicle::Pipeline::create(pipelineInfo);
    }

    void deinit()
    {
        chronicle::Renderer::waitIdle();

        _inFlightFences.clear();
        _renderFinishedSemaphores.clear();
        _imageAvailableSemaphores.clear();
        _commandBuffers.clear();
        _mesh.reset();
        _texture.reset();
        _pipeline.reset();
        _descriptorSets.clear();
        _renderPass.reset();

        chronicle::Gui::deinit();
        chronicle::Renderer::deinit();
        chronicle::Platform::deinit();
    }

    void run()
    {
        while (chronicle::Platform::poll()) {
            chronicle::Renderer::waitForFence(_inFlightFences[_currentFrame]);

            chronicle::Gui::newFrame();

            ImGui::ShowDemoWindow();

            auto imageIndex = chronicle::Renderer::acquireNextImage(_imageAvailableSemaphores[_currentFrame]);
            if (!imageIndex) {
                return;
            }

            chronicle::Renderer::resetFence(_inFlightFences[_currentFrame]);
            _commandBuffers[_currentFrame]->reset();

            recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex.value());
            updateUniformBuffer(_currentFrame);

            chronicle::Renderer::submit(_inFlightFences[_currentFrame], _imageAvailableSemaphores[_currentFrame],
                _renderFinishedSemaphores[_currentFrame], _commandBuffers[_currentFrame]);

            chronicle::Renderer::present(_renderFinishedSemaphores[_currentFrame], imageIndex.value());

            _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

            FrameMark;
        }
    }

    void recordCommandBuffer(const chronicle::CommandBufferRef& commandBuffer, uint32_t imageIndex)
    {
        auto extent = chronicle::Renderer::swapChainExtent();

        commandBuffer->begin();
        commandBuffer->beginRenderPass(_renderPass, chronicle::RectInt2D({ 0, 0 }, extent), imageIndex);

        commandBuffer->setViewport(chronicle::RectFloat2D({ 0.0f, 0.0f },
                                       { static_cast<float>(extent.width), static_cast<float>(extent.height) }),
            0.0f, 1.0f);
        commandBuffer->setScissor(chronicle::RectInt2D({ 0, 0 }, extent));

        commandBuffer->bindPipeline(_pipeline);
        commandBuffer->bindVertexBuffer(_mesh->vertexBuffer(0));
        commandBuffer->bindIndexBuffer(_mesh->indexBuffer(0));
        commandBuffer->bindDescriptorSet(_descriptorSets[_currentFrame], 0);

        commandBuffer->drawIndexed(_mesh->indicesCount(0), 1);

        chronicle::Gui::render(_commandBuffers[_currentFrame]);

        commandBuffer->endRenderPass();
        commandBuffer->end();
    }
    void updateUniformBuffer(uint32_t currentFrame)
    {
        auto extent = chronicle::Renderer::swapChainExtent();

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

private:
    chronicle::RenderPassRef _renderPass;
    std::vector<chronicle::DescriptorSetRef> _descriptorSets = {};
    chronicle::PipelineRef _pipeline;
    std::vector<chronicle::CommandBufferRef> _commandBuffers;
    std::vector<chronicle::SemaphoreRef> _imageAvailableSemaphores;
    std::vector<chronicle::SemaphoreRef> _renderFinishedSemaphores;
    std::vector<chronicle::FenceRef> _inFlightFences;
    uint32_t _currentFrame = 0;

    chronicle::MeshAssetRef _mesh;
    chronicle::TextureAssetRef _texture;
};

int main()
{
    spdlog::set_level(spdlog::level::debug);
    try {
        ExampleApp app {};
        app.init();
        app.run();
        app.deinit();
    } catch (const std::exception& e) {
        CHRLOG_ERROR("Unhandled exception: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}