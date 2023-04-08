#pragma once

#include "pch.h"

#include "Renderer/CommandBuffer.h"
#include "Renderer/Common.h"

namespace chronicle {

class RenderPass;
class Pipeline;
class VertexBuffer;
class IndexBuffer;
class DescriptorSet;

class VulkanCommandBuffer : public CommandBufferI<VulkanCommandBuffer>, private NonCopyable<VulkanCommandBuffer> {
protected:
    explicit VulkanCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool);

public:
    ~VulkanCommandBuffer() = default;

    void resetImpl() const;

    void beginImpl() const;

    void endImpl() const;

    void beginRenderPassImpl(
        const std::shared_ptr<RenderPass>& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const;

    void endRenderPassImpl() const;

    void setViewportImpl(RectFloat2D viewport, float minDepth, float maxDepth) const;

    void setScissorImpl(RectInt2D scissor) const;

    void drawIndexedImpl(uint32_t indexCount, uint32_t instanceCount) const;

    void bindPipelineImpl(const std::shared_ptr<Pipeline>& pipeline);

    void bindVertexBufferImpl(const std::shared_ptr<VertexBuffer>& vertexBuffer) const;

    void bindIndexBufferImpl(const std::shared_ptr<IndexBuffer>& indexBuffer) const;

    void bindDescriptorSetImpl(const std::shared_ptr<DescriptorSet>& descriptorSet, uint32_t index) const;

    [[nodiscard]] inline const vk::CommandBuffer& commandBuffer() const { return _commandBuffer; }

protected:
//    explicit VulkanCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool);

    static std::shared_ptr<CommandBufferI<VulkanCommandBuffer>> createImpl(const Renderer* renderer);

private:
    vk::Device _device;
    vk::CommandPool _commandPool;
    vk::CommandBuffer _commandBuffer;
    vk::PipelineLayout _currentPipelineLayout;

    friend class CommandBufferI;
};

} // namespace chronicle