// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to record command which can be sebsequently submitted to GPU for execution.
/// @tparam T Type with implementation.
template <class T> class CommandBufferI {
public:
    /// @brief Reset the command buffer to the initial state.
    void reset() const { static_cast<const T*>(this)->reset(); }

    /// @brief Start recording a command buffer.
    void begin() const { static_cast<const T*>(this)->begin(); }

    /// @brief End recording a command buffer.
    void end() const { static_cast<const T*>(this)->end(); }

    void beginRenderPass(const RenderPassRef& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
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

    void bindPipeline(const PipelineRef& pipeline) { static_cast<T*>(this)->bindPipeline(pipeline); }
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
    {
        static_cast<const T*>(this)->bindVertexBuffer(vertexBuffer);
    }
    void bindIndexBuffer(const IndexBufferRef& indexBuffer) const
    {
        static_cast<const T*>(this)->bindIndexBuffer(indexBuffer);
    }
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSet(descriptorSet, index);
    }

    static CommandBufferRef create() { return T::create(); }

private:
    CommandBufferI() = default;
    friend T;
};

} // namespace chronicle