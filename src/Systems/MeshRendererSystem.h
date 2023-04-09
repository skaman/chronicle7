#pragma once

#include "pch.h"

#include "Renderer/Renderer.h"
#include "System.h"

namespace chronicle {

class MeshRendererSystem : public System {
public:
    explicit MeshRendererSystem();
    ~MeshRendererSystem() override;
    void run(entt::registry& registry) override;

    void recordCommandBuffer(const CommandBufferRef& commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentFrame);

private:
    RenderPassRef _renderPass;
    std::vector<DescriptorSetRef> _descriptorSets = {};
    PipelineRef _pipeline;
    VertexBufferRef _vertexBuffer;
    IndexBufferRef _indexBuffer;
    std::vector<CommandBufferRef> _commandBuffers;
    std::vector<SemaphoreRef> _imageAvailableSemaphores;
    std::vector<SemaphoreRef> _renderFinishedSemaphores;
    std::vector<FenceRef> _inFlightFences;
    uint32_t _currentFrame = 0;

    ImageRef _texture;
};

} // namespace chronicle