// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "pch.h"

#include <Assets/old/MeshAsset.h>
#include <Assets/old/TextureAsset.h>
#include <Loaders/AssetLoader.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>
#include <Storage/Storage.h>
#include <Utils/Camera.h>
#include <Utils/Scene.h>

#include <imgui/backends/imgui_impl_vulkan.h>

using namespace entt::literals;
using namespace chronicle;

class ExampleApp {
public:
    void init()
    {
        Storage::init();
        Platform::init();
        Renderer::init();

        Platform::dispatcher().sink<CursorPositionEvent>().connect<&ExampleApp::onCursorPosition>(this);
        Platform::dispatcher().sink<MouseButtonEvent>().connect<&ExampleApp::onMouseButton>(this);
        Platform::dispatcher().sink<KeyEvent>().connect<&ExampleApp::onKeyPress>(this);

        CHRLOG_INFO("Start");

        // auto test = AssetLoader::load("D:\\Progetti\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\Sponza.gltf");
        //_mesh2 = test.meshes[0];

        // descriptor sets
        for (auto i = 0; i < Renderer::maxFramesInFlight(); i++) {
            const auto& descriptorSet = Renderer::descriptorSet(i);
            // descriptorSet->addSampler(ShaderStage::fragment, _texture->texture());
            descriptorSet->build();
        }

        _scene = Scene::create("Demo scene");

        _imTexture = ImGui_ImplVulkan_AddTexture(
            _scene->samplerId(), _scene->textureId(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void onCursorPosition(const CursorPositionEvent& evn) { }

    void onMouseButton(const MouseButtonEvent& evn) { }

    void onKeyPress(const KeyEvent& evn) { }

    void deinit()
    {
        Platform::dispatcher().sink<CursorPositionEvent>().disconnect<&ExampleApp::onCursorPosition>(this);
        Platform::dispatcher().sink<MouseButtonEvent>().disconnect<&ExampleApp::onMouseButton>(this);
        Platform::dispatcher().sink<KeyEvent>().disconnect<&ExampleApp::onKeyPress>(this);

        Renderer::waitIdle();

        // ImGui_ImplVulkan_RemoveTexture(_imTexture);

        //_mesh2.reset();
        _scene.reset();

        Renderer::deinit();
        Platform::deinit();
    }

    void run()
    {
        double delta;

        // std::vector<CommandBufferId> commandBuffers(1);

        while (Platform::poll(delta)) {
            if (!Renderer::beginFrame())
                continue;

            _scene->render(Renderer::commandBuffer());

            Renderer::beginRenderPass();

            // commandBuffers[0] = _scene->commandBufferId();

            if (float aspect = static_cast<float>(Renderer::width()) / static_cast<float>(Renderer::height());
                _camera.aspect() != aspect) {
                _camera.setAspect(aspect);
                _camera.recalculateProjection();
            }

            cameraMovements(static_cast<float>(delta));

            drawDebugUI();

            // Renderer::commandBuffer()->beginDebugLabel("Start draw scene", { 0.0f, 1.0f, 0.0f, 1.0f });
            // for (auto i = 0; i < _mesh2->submeshCount(); i++) {
            //     Renderer::commandBuffer()->bindPipeline(_mesh2->pipeline(i));
            //     Renderer::commandBuffer()->bindVertexBuffers(_mesh2->vertexBuffers(i));
            //     Renderer::commandBuffer()->bindIndexBuffer(_mesh2->indexBuffer(i), _mesh2->indexType(i));
            //     Renderer::commandBuffer()->bindDescriptorSet(Renderer::descriptorSet(), 0);
            //     Renderer::commandBuffer()->bindDescriptorSet(_mesh2->material(i)->descriptorSet(), 1);
            //     Renderer::commandBuffer()->drawIndexed(_mesh2->indicesCount(i), 1);
            // }
            // Renderer::commandBuffer()->endDebugLabel();

            // CHRLOG_DEBUG("{}", delta);

            static float time = 0;
            // time += delta;

            _ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            _ubo.view = _camera.view();
            _ubo.proj = _camera.projection();

            Renderer::descriptorSet()->setUniform<UniformBufferObject>("ubo"_hs, _ubo);

            Renderer::endRenderPass();
            Renderer::endFrame();
        }
    }

    void cameraMovements(float delta)
    {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;

        if (!_isMovingCamera
            && (Platform::mouseButton(MouseButton::buttonLeft) == ButtonAction::press
                || Platform::mouseButton(MouseButton::buttonRight) == ButtonAction::press
                || Platform::mouseButton(MouseButton::buttonMiddle) == ButtonAction::press)) {
            _isMovingCamera = true;
            _lastMousePosition = Platform::cursorPosition();
            Platform::setCursorMode(CursorMode::disabled);
        } else if (_isMovingCamera && Platform::mouseButton(MouseButton::buttonLeft) == ButtonAction::release
            && Platform::mouseButton(MouseButton::buttonRight) == ButtonAction::release
            && Platform::mouseButton(MouseButton::buttonMiddle) == ButtonAction::release) {
            _isMovingCamera = false;
            Platform::setCursorMode(CursorMode::normal);
        } else if (_isMovingCamera) {
            if (Platform::key(Key::w) == ButtonAction::press)
                _camera.moveForward(delta);
            if (Platform::key(Key::s) == ButtonAction::press)
                _camera.moveForward(delta * -1.0f);
            if (Platform::key(Key::a) == ButtonAction::press)
                _camera.panHorizontal(delta);
            if (Platform::key(Key::d) == ButtonAction::press)
                _camera.panHorizontal(delta * -1.0f);
            if (Platform::key(Key::q) == ButtonAction::press)
                _camera.panVertical(delta);
            if (Platform::key(Key::e) == ButtonAction::press)
                _camera.panVertical(delta * -1.0f);

            auto currentMousePosition = Platform::cursorPosition();
            auto offsetX = static_cast<float>(currentMousePosition.first - _lastMousePosition.first);
            auto offsetY = static_cast<float>(currentMousePosition.second - _lastMousePosition.second);

            if (Platform::mouseButton(MouseButton::buttonRight) == ButtonAction::press) {
                _camera.rotateHorizontal(offsetX * delta);
                _camera.rotateVertical(offsetY * delta * -1.0f);
            } else if (Platform::mouseButton(MouseButton::buttonMiddle) == ButtonAction::press) {
                _camera.panHorizontal(offsetX * delta * -1.0f);
                _camera.panVertical(offsetY * delta);
            } else if (Platform::mouseButton(MouseButton::buttonLeft) == ButtonAction::press) {
                _camera.rotateHorizontal(offsetX * delta);
                _camera.moveForward(offsetY * delta * -1.0f);
            }

            _camera.recalculateView();
            _lastMousePosition = currentMousePosition;
        }
    }

    void drawDebugUI()
    {
        const ImGuiIO& io = ImGui::GetIO();

        if (!ImGui::Begin("Statistics")) {
            ImGui::End();
            return;
        }

        ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        static bool enabled = false;
        if (ImGui::Checkbox("Show debug lines", &enabled)) {
            Renderer::setDebugShowLines(enabled);
        }
        ImGui::Image(_imTexture, ImVec2 { 1024, 768 });

        ImGui::End();
    }

private:
    SceneRef _scene;
    // MeshRef _mesh2;

    UniformBufferObject _ubo {};

    bool _isMovingCamera = false;
    Camera _camera;

    VkDescriptorSet _imTexture;

    std::pair<double, double> _lastMousePosition;
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