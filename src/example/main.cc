// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "pch.h"

#include <Assets/MeshAsset.h>
#include <Assets/TextureAsset.h>
#include <Platform/Platform.h>
#include <Renderer/Renderer.h>
#include <Utils/Camera.h>

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
        Platform::dispatcher().sink<KeyEvent>().connect<&ExampleApp::onKeyPress>(this);

        CHRLOG_INFO("Start");

        // assets
        _mesh = MeshAsset::load("D:\\viking_room.obj");
        _texture = TextureAsset::load("D:\\viking_room.png");

        // descriptor sets
        for (auto i = 0; i < Renderer::maxFramesInFlight(); i++) {
            const auto& descriptorSet = Renderer::descriptorSet(i);
            descriptorSet->addSampler(ShaderStage::Fragment, _texture->texture());
            descriptorSet->build();
        }

        // pipeline
        PipelineInfo pipelineInfo = {};
        pipelineInfo.shaders[ShaderStage::Vertex] = "Shaders/triangle.vert.bin";
        pipelineInfo.shaders[ShaderStage::Fragment] = "Shaders/triangle.frag.bin";
        pipelineInfo.vertexBuffers.push_back(_mesh->bufferInfo());

        _pipeline = Pipeline::create(pipelineInfo, "Test pipeline");
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

            if (float aspect = static_cast<float>(Renderer::width()) / static_cast<float>(Renderer::height());
                _camera.aspect() != aspect) {
                _camera.setAspect(aspect);
                _camera.recalculateProjection();
            }

            cameraMovements(static_cast<float>(delta));

            drawDebugUI();
            // drawCamera();

            Renderer::commandBuffer()->beginDebugLabel("Start draw scene", { 0.0f, 1.0f, 0.0f, 1.0f });
            Renderer::commandBuffer()->bindPipeline(_pipeline);
            Renderer::commandBuffer()->bindVertexBuffer(_mesh->vertexBuffer(0));
            Renderer::commandBuffer()->bindIndexBuffer(_mesh->indexBuffer(0));
            Renderer::commandBuffer()->bindDescriptorSet(Renderer::descriptorSet(), 0);
            Renderer::commandBuffer()->drawIndexed(_mesh->indicesCount(0), 1);
            Renderer::commandBuffer()->endDebugLabel();

            // CHRLOG_DEBUG("{}", delta);

            static float time = 0;
            // time += delta;

            _ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            _ubo.view = _camera.view();
            _ubo.proj = _camera.projection();

            Renderer::descriptorSet()->setUniform<UniformBufferObject>("ubo"_hs, _ubo);

            Renderer::endFrame();
        }
    }

    void cameraMovements(float delta)
    {
        if (!_isMovingCamera && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)
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

        ImGui::End();
    }

    // void drawCamera()
    //{
    //     const ImGuiIO& io = ImGui::GetIO();
    //
    //     if (!ImGui::Begin("Camera")) {
    //         ImGui::End();
    //         return;
    //     }
    //
    //     glm::vec3 scale;
    //     glm::quat rotation;
    //     glm::vec3 translation;
    //     glm::vec3 skew;
    //     glm::vec4 perspective;
    //     glm::decompose(_ubo.view, scale, rotation, translation, skew, perspective);
    //
    //     if (ImGui::InputFloat3("Translation", glm::value_ptr(translation))) {
    //         _ubo.view = glm::translate(translation) * glm::toMat4(rotation) * glm::scale(scale);
    //     }
    //     ImGui::InputFloat3("Rotation", glm::value_ptr(skew));
    //     ImGui::InputFloat4("Rotation2", glm::value_ptr(rotation));
    //     if (ImGui::InputFloat3("Scale", glm::value_ptr(scale))) {
    //         _ubo.view = glm::translate(translation) * glm::toMat4(rotation) * glm::scale(scale);
    //     }
    //
    //     ImGui::End();
    // }

private:
    PipelineRef _pipeline;
    MeshAssetRef _mesh;
    TextureAssetRef _texture;

    UniformBufferObject _ubo {};

    bool _isMovingCamera = false;
    Camera _camera;

    std::pair<double, double> _lastMousePosition;

    glm::vec3 _cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
    glm::vec3 _cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
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