#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class CommandBufferI {
public:
    void reset() const { static_cast<const T*>(this)->reset(); }

    void begin() const { static_cast<const T*>(this)->begin(); }

    void end() const { static_cast<const T*>(this)->end(); }

    void beginRenderPass(
        const std::shared_ptr<RenderPass>& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
    {
        static_cast<const T*>(this)->beginRenderPass(renderPass, renderArea, imageIndex);
    }

    void endRenderPass() const { static_cast<const T*>(this)->endRenderPass(); }

    void setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
    {
        static_cast<const T*>(this)->setViewport(viewport, minDepth, maxDepth);
    }

    void setScissor(RectInt2D scissor) const { static_cast<const T*>(this)->setScissor(scissor); }

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
    {
        static_cast<const T*>(this)->drawIndexed(indexCount, instanceCount);
    }

    void bindPipeline(const std::shared_ptr<Pipeline>& pipeline) { static_cast<T*>(this)->bindPipeline(pipeline); }
    void bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) const
    {
        static_cast<const T*>(this)->bindVertexBuffer(vertexBuffer);
    }
    void bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) const
    {
        static_cast<const T*>(this)->bindIndexBuffer(indexBuffer);
    }
    void bindDescriptorSet(const std::shared_ptr<DescriptorSet>& descriptorSet, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSet(descriptorSet, index);
    }

    static CommandBufferRef create(const Renderer* renderer) { return T::create(renderer); }

private:
    CommandBufferI() = default;
    friend T;
};

} // namespace chronicle