// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Scene.h"

using namespace entt::literals;

namespace chronicle {

CHR_CONCRETE(Scene);

Scene::Scene(const std::string& name)
    : _name(name)
{
    CHRZONE_SCENE;

    // create command buffers
    //_commandBuffers.resize(Renderer::maxFramesInFlight());
    // for (auto i = 0; i < Renderer::maxFramesInFlight(); i++) {
    //    auto debugName = std::format("Scene {} command buffer (frame {})", _name, i);
    //    _commandBuffers[i] = VulkanCommandBuffer::create(debugName.c_str());
    //}

    _imageFormat = Renderer::swapChainImageFormat();
    _depthFormat = Renderer::findDepthFormat();
    _msaa = MSAA::sampleCount8;
    _width = 1024;
    _height = 768;

    // TODO: handle MSAA and resolution
    _colorTexture = Texture::createColor({ .width = _width,
                                             .height = _height,
                                             .format = _imageFormat,
                                             .msaa = _msaa,
                                             .isInputAttachment = true, // TODO: maybe false?
                                             .generateMipmaps = false },
        fmt::format("Color texture for sene {}", _name));
    _depthTexture = Texture::createDepth({ .width = _width, .height = _height, .format = _depthFormat, .msaa = _msaa },
        fmt::format("Depth texture for sene {}", _name));
    _resolveTexture = Texture::createColor({ .width = _width,
                                               .height = _height,
                                               .format = _imageFormat,
                                               .msaa = MSAA::sampleCount1,
                                               .isInputAttachment = true,
                                               .generateMipmaps = false },
        fmt::format("Resolve texture for sene {}", _name));

    // color attachment
    RenderPassAttachment colorAttachment = { .format = _imageFormat,
        .msaa = _msaa,
        .loadOp = AttachmentLoadOp::clear,
        .storeOp = AttachmentStoreOp::store,
        .stencilLoadOp = AttachmentLoadOp::dontCare,
        .stencilStoreOp = AttachmentStoreOp::dontCare,
        .initialLayout = ImageLayout::undefined,
        .finalLayout = ImageLayout::colorAttachment };

    // depth attachment
    RenderPassAttachment depthAttachment = { .format = _depthFormat,
        .msaa = _msaa,
        .loadOp = AttachmentLoadOp::clear,
        .storeOp = AttachmentStoreOp::dontCare,
        .stencilLoadOp = AttachmentLoadOp::dontCare,
        .stencilStoreOp = AttachmentStoreOp::dontCare,
        .initialLayout = ImageLayout::undefined,
        .finalLayout = ImageLayout::depthStencilAttachment };

    // resolve attachment
    RenderPassAttachment resolveAttachment = { .format = _imageFormat,
        .msaa = MSAA::sampleCount1,
        .loadOp = AttachmentLoadOp::dontCare,
        .storeOp = AttachmentStoreOp::store,
        .stencilLoadOp = AttachmentLoadOp::dontCare,
        .stencilStoreOp = AttachmentStoreOp::dontCare,
        .initialLayout = ImageLayout::undefined,
        .finalLayout = ImageLayout::shaderReadOnly };

    // create render pass
    _renderPass = RenderPass::create({ .colorAttachment = colorAttachment,
                                         .depthStencilAttachment = depthAttachment,
                                         .resolveAttachment = resolveAttachment },
        fmt::format("Render pass for scene {}", _name));

    // create frame buffer
    FrameBufferInfo frameBufferInfo = {};
    frameBufferInfo.attachments
        = { _colorTexture->textureId(), _depthTexture->textureId(), _resolveTexture->textureId() };
    frameBufferInfo.renderPass = _renderPass->renderPassId();
    frameBufferInfo.width = _width;
    frameBufferInfo.height = _height;
    _frameBuffer = FrameBuffer::create(frameBufferInfo, fmt::format("Frambuffer for scene {}", _name));

    // TODO: test to remove
    auto test = AssetLoader::load("D:\\Progetti\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\Sponza.gltf", _renderPass);
    _mesh = test.meshes[0];
}

void Scene::render(CommandBufferRef commandBuffer)
{
    // start
    auto currentFrame = Renderer::currentFrame();

    // begin render pass
    // TODO: add local cache for renderpass id and frame buffer id
    commandBuffer->beginRenderPass({ .renderPassId = _renderPass->renderPassId(),
        .frameBufferId = _frameBuffer->frameBufferId(),
        .renderAreaOffset = { 0, 0 },
        .renderAreaExtent = { _width, _height } });

    // set viewport
    commandBuffer->setViewport({ .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(_width),
        .height = static_cast<float>(_height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f });

    // camera
    if (float aspect = static_cast<float>(_width) / static_cast<float>(_height); _camera.aspect() != aspect) {
        _camera.setAspect(aspect);
        _camera.recalculateProjection();
    }

    // draw
    commandBuffer->beginDebugLabel("Start draw scene", { 0.0f, 1.0f, 0.0f, 1.0f });
    for (auto i = 0; i < _mesh->submeshCount(); i++) {
        commandBuffer->bindPipeline(_mesh->pipeline(i)->pipelineId());
        commandBuffer->bindVertexBuffers(_mesh->vertexBufferIds(i), _mesh->vertexBufferOffsets(i));
        commandBuffer->bindIndexBuffer(_mesh->indexBufferId(i), _mesh->indexType(i));
        commandBuffer->bindDescriptorSet(
            Renderer::descriptorSet()->descriptorSetId(), _mesh->pipeline(i)->pipelineLayoutId(), 0);
        commandBuffer->bindDescriptorSet(
            _mesh->material(i)->descriptorSet()->descriptorSetId(), _mesh->pipeline(i)->pipelineLayoutId(), 1);
        commandBuffer->drawIndexed(_mesh->indicesCount(i), 1);
    }
    commandBuffer->endDebugLabel();

    // descriptor set
    _ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    _ubo.view = _camera.view();
    _ubo.proj = _camera.projection();

    Renderer::descriptorSet()->setUniform<UniformBufferObject>("ubo"_hs, _ubo);

    // end
    commandBuffer->endRenderPass();
}

SceneRef Scene::create(const std::string& name)
{
    CHRZONE_SCENE;

    // create an instance of the class
    return std::make_shared<ConcreteScene>(name);
}

} // namespace chronicle