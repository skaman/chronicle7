#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

class Renderer;
class DescriptorSet;
class IndexBuffer;
class Pipeline;
class RenderPass;
class VertexBuffer;

template <class T> class CommandBufferI;

#ifdef VULKAN_RENDERER
class VulkanCommandBuffer;
using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;

template <class T> class CommandBufferI {
public:
    void reset() const { static_cast<const T*>(this)->resetImpl(); }

    void begin() const { static_cast<const T*>(this)->beginImpl(); }

    void end() const { static_cast<const T*>(this)->endImpl(); }

    void beginRenderPass(
        const std::shared_ptr<RenderPass>& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
    {
        static_cast<const T*>(this)->beginRenderPassImpl(renderPass, renderArea, imageIndex);
    }

    void endRenderPass() const { static_cast<const T*>(this)->endRenderPassImpl(); }

    void setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
    {
        static_cast<const T*>(this)->setViewportImpl(viewport, minDepth, maxDepth);
    }

    void setScissor(RectInt2D scissor) const { static_cast<const T*>(this)->setScissorImpl(scissor); }

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
    {
        static_cast<const T*>(this)->drawIndexedImpl(indexCount, instanceCount);
    }

    void bindPipeline(const std::shared_ptr<Pipeline>& pipeline)
    {
        static_cast<T*>(this)->bindPipelineImpl(pipeline);
    }
    void bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) const
    {
        static_cast<const T*>(this)->bindVertexBufferImpl(vertexBuffer);
    }
    void bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) const
    {
        static_cast<const T*>(this)->bindIndexBufferImpl(indexBuffer);
    }
    void bindDescriptorSet(const std::shared_ptr<DescriptorSet>& descriptorSet, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSetImpl(descriptorSet, index);
    }

    static std::shared_ptr<CommandBufferI<T>> create(const Renderer* renderer) { return T::createImpl(renderer); }
};

} // namespace chronicle