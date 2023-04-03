#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanCommandBuffer.h"
#endif

namespace chronicle {

class Renderer;

class CommandBuffer {
public:
    explicit CommandBuffer(const Renderer* renderer);

    void reset() const { _commandBuffer.reset(); }

    void begin() const { _commandBuffer.begin(); }
    void end() const { _commandBuffer.end(); }

    void beginRenderPass(
        const std::shared_ptr<RenderPass>& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
    {
        _commandBuffer.beginRenderPass(renderPass, renderArea, imageIndex);
    }
    void endRenderPass() const { _commandBuffer.endRenderPass(); }

    void setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
    {
        _commandBuffer.setViewport(viewport, minDepth, maxDepth);
    }
    void setScissor(RectInt2D scissor) const { _commandBuffer.setScissor(scissor); }

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
    {
        _commandBuffer.drawIndexed(indexCount, instanceCount);
    }

    void bindPipeline(const std::shared_ptr<Pipeline>& pipeline) { _commandBuffer.bindPipeline(pipeline); }
    void bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) const
    {
        _commandBuffer.bindVertexBuffer(vertexBuffer);
    }
    void bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) const
    {
        _commandBuffer.bindIndexBuffer(indexBuffer);
    }
    void bindDescriptorSet(const std::shared_ptr<DescriptorSet>& descriptorSet, uint32_t index) const
    {
        _commandBuffer.bindDescriptorSet(descriptorSet, index);
    }

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanCommandBuffer& native() const { return _commandBuffer; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanCommandBuffer _commandBuffer;
#endif
};

} // namespace chronicle