#pragma once

#include "pch.h"

#include "Renderer/CommandBufferI.h"

namespace chronicle {

class VulkanCommandBuffer : public CommandBufferI<VulkanCommandBuffer>, private NonCopyable<VulkanCommandBuffer> {
protected:
    explicit VulkanCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool);

public:
    ~VulkanCommandBuffer() = default;

    [[nodiscard]] const vk::CommandBuffer& commandBuffer() const { return _commandBuffer; }

    void reset() const;
    void begin() const;
    void end() const;
    void beginRenderPass(const RenderPassRef& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const;
    void endRenderPass() const;
    void setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const;
    void setScissor(RectInt2D scissor) const;
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const;
    void bindPipeline(const PipelineRef& pipeline);
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const;
    void bindIndexBuffer(const IndexBufferRef& indexBuffer) const;
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const;

    static CommandBufferRef create(const Renderer* renderer);

private:
    vk::Device _device;
    vk::CommandPool _commandPool;
    vk::CommandBuffer _commandBuffer;
    vk::PipelineLayout _currentPipelineLayout;
};

} // namespace chronicle