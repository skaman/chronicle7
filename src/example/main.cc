// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "pch.h"

#include <Assets/MeshAsset.h>
#include <Assets/TextureAsset.h>
#include <Gui/Gui.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>

using namespace entt::literals;

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

        for (const auto& descriptorSet : chronicle::Renderer::descriptorSets()) {
            descriptorSet->addSampler(chronicle::ShaderStage::Fragment, _texture->texture());
            descriptorSet->build();
        }

        // just after the render pass creation
        // chronicle::Gui::init(2, _renderPass);

        // pipeline
        chronicle::PipelineInfo pipelineInfo = {};
        pipelineInfo.shaders[chronicle::ShaderStage::Vertex] = "Shaders/triangle.vert.bin";
        pipelineInfo.shaders[chronicle::ShaderStage::Fragment] = "Shaders/triangle.frag.bin";
        pipelineInfo.vertexBuffers.push_back(_mesh->bufferInfo());

        _pipeline = chronicle::Pipeline::create(pipelineInfo);
    }

    void deinit()
    {
        chronicle::Renderer::waitIdle();

        _mesh.reset();
        _texture.reset();
        _pipeline.reset();

        // chronicle::Gui::deinit();
        chronicle::Renderer::deinit();
        chronicle::Platform::deinit();
    }

    void run()
    {
        while (chronicle::Platform::poll()) {
            if (!chronicle::Renderer::beginFrame())
                continue;

            chronicle::Renderer::commandBuffer()->bindPipeline(_pipeline);
            chronicle::Renderer::commandBuffer()->bindVertexBuffer(_mesh->vertexBuffer(0));
            chronicle::Renderer::commandBuffer()->bindIndexBuffer(_mesh->indexBuffer(0));
            chronicle::Renderer::commandBuffer()->bindDescriptorSet(chronicle::Renderer::descriptorSet(), 0);
            chronicle::Renderer::commandBuffer()->drawIndexed(_mesh->indicesCount(0), 1);

            float time = 0;

            chronicle::UniformBufferObject ubo {};
            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view
                = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f),
                static_cast<float>(chronicle::Renderer::width()) / static_cast<float>(chronicle::Renderer::height()),
                0.1f, 10.0f);
            ubo.proj[1][1] *= -1;

            chronicle::Renderer::descriptorSet()->setUniform<chronicle::UniformBufferObject>("ubo"_hs, ubo);

            chronicle::Renderer::endFrame();
        }
    }

private:
    chronicle::PipelineRef _pipeline;
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