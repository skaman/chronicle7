// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Structure specifying a viewport.
struct Viewport {
public:
    float x; ///< The viewport upper left corner X.
    float y; ///< The viewport upper left corner Y.
    float width; ///< The viewport width.
    float height; ///< The viewport height.
    float minDepth; ///< The viewport min depth.
    float maxDepth; ///< The viewport max depth.
};

/// @brief Structure specifying render pass begin information.
struct RenderPassBeginInfo {
    RenderPassId renderPassId; ///< The render pass to begin an instance of.
    FrameBufferId frameBufferId; ///< The framebuffer containing the attachments that are used with the render pass.
    glm::i32vec2 renderAreaOffset; ///< The offset for the render area that is affected by the render pass instance.
    glm::u32vec2 renderAreaExtent; ///< The extent for the render area that is affected by the render pass instance.
};

/// @brief Object used to record command which can be sebsequently submitted to GPU for execution.
/// @tparam T Type with implementation.
template <class T> class CommandBufferI {
public:
    /// @brief Start recording the command buffer.
    void begin() const { static_cast<const T*>(this)->begin(); }

    /// @brief End recording the command buffer.
    void end() const { static_cast<const T*>(this)->end(); }

    /// @brief Set the viewport for the command buffer.
    /// @param viewport Structure specifying a viewport.
    void setViewport(const Viewport& viewport) const { static_cast<const T*>(this)->setViewport(viewport); }

    /// @brief Begin a new render pass.
    /// @param renderPassInfo Structure specifying render pass begin information.
    void beginRenderPass(const RenderPassBeginInfo& renderPassInfo) const
    {
        static_cast<const T*>(this)->beginRenderPass(renderPassInfo);
    }

    /// @brief End the render pass.
    void endRenderPass() const { static_cast<const T*>(this)->endRenderPass(); }

    /// @brief Draw primitives with indexed vertices.
    /// @param indexCount The number of vertices to draw.
    /// @param instanceCount The number of instances to draw.
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
    {
        static_cast<const T*>(this)->drawIndexed(indexCount, instanceCount);
    }

    /// @brief Bind a pipeline object to the command buffer.
    /// @param pipeline The pipeline to be bound.
    void bindPipeline(const PipelineRef& pipeline) { static_cast<T*>(this)->bindPipeline(pipeline); }

    /// @brief Bind a vertex buffer to the command buffer.
    /// @param vertexBuffer The vertex buffer to be bound.
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
    {
        static_cast<const T*>(this)->bindVertexBuffer(vertexBuffer);
    }

    /// @brief Bind a group of vertex buffers to the command buffer.
    /// @param vertexBuffer The vertex buffers to be bound.
    void bindVertexBuffers(const VertexBuffersRef& vertexBuffers) const
    {
        static_cast<const T*>(this)->bindVertexBuffers(vertexBuffers);
    }

    /// @brief Bind an index buffer to the command buffer.
    /// @param indexBuffer The index buffer to be bound.
    /// @param indexType Specify the size of the index.
    void bindIndexBuffer(const IndexBufferRef& indexBuffer, IndexType indexType) const
    {
        static_cast<const T*>(this)->bindIndexBuffer(indexBuffer, indexType);
    }

    /// @brief Bind a descriptor set to the command buffer.
    /// @param descriptorSet The descriptor set to be bound.
    /// @param index The number of the descriptor to be bound.
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSet(descriptorSet, index);
    }

    /// @brief Begin a debug label.
    /// @param name Label name.
    /// @param color Label color.
    void beginDebugLabel(const char* name, glm::vec4 color)
    {
        static_cast<const T*>(this)->beginDebugLabel(name, color);
    }

    /// @brief End a debug label.
    void endDebugLabel() const { static_cast<const T*>(this)->endDebugLabel(); }

    /// @brief Insert a debug label.
    /// @param name Label name.
    /// @param color Label color.
    void insertDebugLabel(const char* name, glm::vec4 color) const
    {
        static_cast<const T*>(this)->insertDebugLabel(name, color);
    }

    /// @brief Get the command buffer handle ID
    /// @return Command buffer ID
    [[nodiscard]] CommandBufferId commandBufferId() const { return static_cast<const T*>(this)->commandBufferId(); }

private:
    CommandBufferI() = default;
    friend T;
};

} // namespace chronicle