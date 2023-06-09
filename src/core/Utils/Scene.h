// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Camera.h"
#include "Loaders/AssetLoader.h"
#include "Renderer/Renderer.h"

namespace chronicle {

class Scene;
using SceneRef = std::shared_ptr<Scene>;

class Scene {
protected:
    explicit Scene(const std::string& name);

public:
    void render(CommandBufferRef commandBuffer);

    static SceneRef create(const std::string& name);

    [[nodiscard]] TextureId textureId() const { return _resolveTexture->textureId(); }
    [[nodiscard]] SamplerId samplerId() const { return _resolveTexture->samplerId(); }

    //[[nodiscard]] CommandBufferId commandBufferId() const
    //{
    //    return _commandBuffers[Renderer::currentFrame()]->commandBufferId();
    //}

private:
    std::string _name = {};
    // std::vector<CommandBufferRef> _commandBuffers = {};

    Format _imageFormat = {};
    Format _depthFormat = {};
    MSAA _msaa = {};
    uint32_t _width = 0;
    uint32_t _height = 0;

    TextureRef _colorTexture = {};
    TextureRef _depthTexture = {};
    TextureRef _resolveTexture = {};
    RenderPassRef _renderPass = {};
    FrameBufferRef _frameBuffer = {};

    MeshRef _mesh = {};
    internal::vulkan::UniformBufferObject _ubo {};
    Camera _camera;
};

} // namespace chronicle