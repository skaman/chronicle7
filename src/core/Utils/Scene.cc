// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Scene.h"

namespace chronicle {

CHR_CONCRETE(Scene);

Scene::Scene(const std::string& name)
    : _name(name)
{
    CHRZONE_SCENE;

    // create command buffers
    _commandBuffers.resize(Renderer::maxFramesInFlight());
    for (auto i = 0; i < Renderer::maxFramesInFlight(); i++) {
        auto debugName = std::format("Scene {} command buffer (frame {})", _name, i);
        _commandBuffers[i] = VulkanCommandBuffer::create(debugName.c_str());
    }

    _imageFormat = Renderer::swapChainImageFormat();
    _depthFormat = Renderer::findDepthFormat();
    _msaa = MSAA::sampleCount1;
    _width = 1024;
    _height = 768;

    // TODO: handle MSAA and resolution
    _colorTexture = Texture::createColor({ .width = _width, .height = _height, .format = _imageFormat, .msaa = _msaa });
    _depthTexture = Texture::createDepth({ .width = _width, .height = _height, .format = _depthFormat, .msaa = _msaa });

    // create render pass

    // color attachment
    RenderPassAttachment colorAttachment = { .format = _imageFormat,
        .msaa = _msaa,
        .loadOp = AttachmentLoadOp::clear,
        .storeOp = AttachmentStoreOp::store,
        .stencilLoadOp = AttachmentLoadOp::dontCare,
        .stencilStoreOp = AttachmentStoreOp::dontCare,
        .initialLayout = ImageLayout::undefined,
        .finalLayout = ImageLayout::shaderReadOnly };

    // depth attachment
    RenderPassAttachment depthAttachment = { .format = _depthFormat,
        .msaa = _msaa,
        .loadOp = AttachmentLoadOp::clear,
        .storeOp = AttachmentStoreOp::dontCare,
        .stencilLoadOp = AttachmentLoadOp::dontCare,
        .stencilStoreOp = AttachmentStoreOp::dontCare,
        .initialLayout = ImageLayout::undefined,
        .finalLayout = ImageLayout::depthStencilAttachment };

    _renderPass = RenderPass::create({ .colorAttachment = colorAttachment, .depthStencilAttachment = depthAttachment });

    // create frame buffer
    FrameBufferInfo frameBufferInfo = {};
    frameBufferInfo.attachments = { _colorTexture->textureId(), _depthTexture->textureId() };
    frameBufferInfo.renderPass = _renderPass->renderPassId();
    frameBufferInfo.width = _width;
    frameBufferInfo.height = _height;
    _frameBuffer = FrameBuffer::create(frameBufferInfo);
}

void Scene::render() { }

SceneRef Scene::create(const std::string& name)
{
    CHRZONE_SCENE;

    // create an instance of the class
    return std::make_shared<ConcreteScene>(name);
}

} // namespace chronicle