// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "pch.h"

#include <Assets/MeshAsset.h>
#include <Assets/TextureAsset.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>

using namespace entt::literals;
using namespace chronicle;

class ExampleApp {
public:
    void init()
    {
        Platform::init();
        Renderer::init();

        Platform::dispatcher().sink<CursorPositionEvent>().connect<&ExampleApp::onCursorPosition>(this);
        Platform::dispatcher().sink<MouseButtonEvent>().connect<&ExampleApp::onMouseButton>(this);

        CHRLOG_INFO("Start");

        // assets
        _mesh = MeshAsset::load("D:\\viking_room.obj");
        _texture = TextureAsset::load("D:\\viking_room.png");

        // descriptor sets
        for (const auto& descriptorSet : Renderer::descriptorSets()) {
            descriptorSet->addSampler(ShaderStage::Fragment, _texture->texture());
            descriptorSet->build();
        }

        // pipeline
        PipelineInfo pipelineInfo = {};
        pipelineInfo.shaders[ShaderStage::Vertex] = "Shaders/triangle.vert.bin";
        pipelineInfo.shaders[ShaderStage::Fragment] = "Shaders/triangle.frag.bin";
        pipelineInfo.vertexBuffers.push_back(_mesh->bufferInfo());

        _pipeline = Pipeline::create(pipelineInfo);
    }

    void onCursorPosition(const CursorPositionEvent& evn) const { }
    void onMouseButton(const MouseButtonEvent& evn) const
    {
        CHRLOG_DEBUG("{} {} {}", (int)evn.data.action, (int)evn.data.button, (int)evn.data.modifier);

        if (evn.data.action == ButtonAction::press) {
            Platform::setCursorType(CursorType::hand);
        } else {
            Platform::setCursorType(CursorType::arrow);
        }
    }

    void deinit()
    {
        Platform::dispatcher().sink<CursorPositionEvent>().disconnect<&ExampleApp::onCursorPosition>(this);
        Platform::dispatcher().sink<MouseButtonEvent>().disconnect<&ExampleApp::onMouseButton>(this);

        Renderer::waitIdle();

        _mesh.reset();
        _texture.reset();
        _pipeline.reset();

        Renderer::deinit();
        Platform::deinit();
    }

    void run()
    {
        double delta;
        while (Platform::poll(delta)) {
            if (!Renderer::beginFrame())
                continue;

            drawDebugUI(delta);
            // ImGui::ShowDemoWindow();
            //ImGui::ShowMetricsWindow();

            Renderer::commandBuffer()->bindPipeline(_pipeline);
            Renderer::commandBuffer()->bindVertexBuffer(_mesh->vertexBuffer(0));
            Renderer::commandBuffer()->bindIndexBuffer(_mesh->indexBuffer(0));
            Renderer::commandBuffer()->bindDescriptorSet(Renderer::descriptorSet(), 0);
            Renderer::commandBuffer()->drawIndexed(_mesh->indicesCount(0), 1);

            //CHRLOG_DEBUG("{}", delta);

            float time = 0;

            UniformBufferObject ubo {};
            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view
                = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f),
                static_cast<float>(Renderer::width()) / static_cast<float>(Renderer::height()), 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;

            Renderer::descriptorSet()->setUniform<UniformBufferObject>("ubo"_hs, ubo);

            Renderer::endFrame();
        }
    }

    void drawDebugUI(double delta) {
        if (!ImGui::Begin("Statistics")) {
            ImGui::End();
            return;
        }

        ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", delta * 1000, 1.0 / delta);

        ImGui::End();
    }

private:
    PipelineRef _pipeline;
    MeshAssetRef _mesh;
    TextureAssetRef _texture;
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