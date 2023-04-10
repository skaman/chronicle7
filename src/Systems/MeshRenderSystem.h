#pragma once

#include "pch.h"

#include "Assets/MeshAsset.h"
#include "Assets/TextureAsset.h"
#include "Renderer/Renderer.h"
#include "System.h"

namespace chronicle {

class MeshRenderSystem : public System {
public:
    explicit MeshRenderSystem();
    ~MeshRenderSystem() override;
    void activate() override;
    void deactivate() override;
    void run(entt::registry& registry) override;

    void recordCommandBuffer(const CommandBufferRef& commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentFrame);

private:
    RenderPassRef _renderPass;
    std::vector<DescriptorSetRef> _descriptorSets = {};
    PipelineRef _pipeline;
    std::vector<CommandBufferRef> _commandBuffers;
    std::vector<SemaphoreRef> _imageAvailableSemaphores;
    std::vector<SemaphoreRef> _renderFinishedSemaphores;
    std::vector<FenceRef> _inFlightFences;
    uint32_t _currentFrame = 0;

    MeshAssetRef _mesh;
    TextureAssetRef _texture;
};

} // namespace chronicle