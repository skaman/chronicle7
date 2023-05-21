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
    /// @brief Get the name for the command buffer.
    /// @return Command buffer name.
    [[nodiscard]] std::string name() const { static_cast<const T*>(this)->name(); }

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
    /// @param pipelineId The pipeline to be bound.
    void bindPipeline(PipelineId pipelineId) const { static_cast<const T*>(this)->bindPipeline(pipelineId); }

    /// @brief Bind a vertex buffer to the command buffer.
    /// @param vertexBufferId The vertex buffer to be bound.
    /// @param offset The vertex buffer data offset.
    void bindVertexBuffer(VertexBufferId vertexBufferId, uint64_t offset = 0) const
    {
        static_cast<const T*>(this)->bindVertexBuffer(vertexBufferId, offset);
    }

    /// @brief Bind a group of vertex buffers to the command buffer.
    /// @param vertexBuffers The vertex buffers to be bound.
    /// @param offsets The vertex buffers data offset.
    void bindVertexBuffers(const std::vector<VertexBufferId>& vertexBuffers, const std::vector<uint64_t>& offsets) const
    {
        static_cast<const T*>(this)->bindVertexBuffers(vertexBuffers, offsets);
    }

    /// @brief Bind an index buffer to the command buffer.
    /// @param indexBufferId The index buffer to be bound.
    /// @param indexType Specify the size of the index.
    /// @param offset The index buffer data offset.
    void bindIndexBuffer(IndexBufferId indexBufferId, IndexType indexType, uint64_t offset = 0) const
    {
        static_cast<const T*>(this)->bindIndexBuffer(indexBufferId, indexType, offset);
    }

    /// @brief Bind a descriptor set to the command buffer.
    /// @param descriptorSetId The descriptor set to be bound.
    /// @param pipelineLayoutId The pipeline layout related to the descript set.
    /// @param index The number of the descriptor to be bound.
    void bindDescriptorSet(DescriptorSetId descriptorSetId, PipelineLayoutId pipelineLayoutId, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSet(descriptorSetId, pipelineLayoutId, index);
    }

    /// @brief Begin a debug label.
    /// @param name Label name.
    /// @param color Label color.
    void beginDebugLabel(const std::string& name, glm::vec4 color) const
    {
        static_cast<const T*>(this)->beginDebugLabel(name, color);
    }

    /// @brief End a debug label.
    void endDebugLabel() const { static_cast<const T*>(this)->endDebugLabel(); }

    /// @brief Insert a debug label.
    /// @param name Label name.
    /// @param color Label color.
    void insertDebugLabel(const std::string& name, glm::vec4 color) const
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