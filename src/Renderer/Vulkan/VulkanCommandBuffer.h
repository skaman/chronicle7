#pragma once

#include "pch.h"

#include "Renderer/Common.h"

namespace chronicle {

class RenderPass;
class Pipeline;
class VertexBuffer;
class IndexBuffer;
class DescriptorSet;

class VulkanCommandBuffer {
public:
    explicit VulkanCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool);

    void reset() const;

    void begin() const;
    void end() const;

    void beginRenderPass(
        const std::shared_ptr<RenderPass>& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const;
    void endRenderPass() const;

    void setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const;
    void setScissor(RectInt2D scissor) const;

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const;

    void bindPipeline(const std::shared_ptr<Pipeline>& pipeline);
    void bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) const;
    void bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) const;
    void bindDescriptorSet(const std::shared_ptr<DescriptorSet>& descriptorSet, uint32_t index) const;

    // internal
    [[nodiscard]] inline const vk::CommandBuffer& commandBuffer() const { return _commandBuffer; }

private:
    vk::Device _device;
    vk::CommandPool _commandPool;

    vk::CommandBuffer _commandBuffer;

    vk::PipelineLayout _currentPipelineLayout;
};

} // namespace chronicle