#pragma once

#include "pch.h"

//#include "Renderer/Common.h"
#include "Renderer/Renderer.h"
// #include "Renderer/Vulkan/VulkanCommandBuffer.h"
//#include "Renderer/CommandBuffer.h"
#include "System.h"

namespace chronicle {

class MeshRendererSystem : public System {
public:
    explicit MeshRendererSystem();
    ~MeshRendererSystem() override;
    void run(entt::registry& registry) override;

    void recordCommandBuffer(const std::shared_ptr<CommandBuffer>& commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentFrame);

private:
    std::shared_ptr<RenderPass> _renderPass;
    std::vector<std::shared_ptr<DescriptorSet>> _descriptorSets = {};
    std::shared_ptr<Pipeline> _pipeline;
    std::shared_ptr<VertexBuffer> _vertexBuffer;
    std::shared_ptr<IndexBuffer> _indexBuffer;
    std::vector<std::shared_ptr<CommandBuffer>> _commandBuffers;
    std::vector<std::shared_ptr<Semaphore>> _imageAvailableSemaphores;
    std::vector<std::shared_ptr<Semaphore>> _renderFinishedSemaphores;
    std::vector<FenceRef> _inFlightFences;
    uint32_t _currentFrame = 0;

    std::shared_ptr<Image> _texture;
};

} // namespace chronicle