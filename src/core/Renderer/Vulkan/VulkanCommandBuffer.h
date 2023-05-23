// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseCommandBuffer.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref BaseCommandBuffer
class VulkanCommandBuffer : public BaseCommandBuffer<VulkanCommandBuffer>, private NonCopyable<VulkanCommandBuffer> {
protected:
    /// @brief Default constructor.
    /// @param debugName Debug name.
    explicit VulkanCommandBuffer(const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanCommandBuffer() = default;

    /// @brief @see BaseCommandBuffer#name
    [[nodiscard]] std::string name() const { return _name; }

    /// @brief @see BaseCommandBuffer#begin
    void begin() const;

    /// @brief @see BaseCommandBuffer#end
    void end() const;

    /// @brief @see BaseCommandBuffer#setViewport
    void setViewport(const ViewportInfo& viewport) const;

    /// @brief @see BaseCommandBuffer#beginRenderPass
    void beginRenderPass(const RenderPassBeginInfo& renderPassInfo) const;

    /// @brief @see BaseCommandBuffer#endRenderPass
    void endRenderPass() const;

    /// @brief @see BaseCommandBuffer#drawIndexed
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const;

    /// @brief @see BaseCommandBuffer#bindPipeline
    void bindPipeline(PipelineId pipelineId) const;

    /// @brief @see BaseCommandBuffer#bindVertexBuffer
    void bindVertexBuffer(VertexBufferId vertexBufferId, uint64_t offset) const;

    /// @brief @see BaseCommandBuffer#bindVertexBuffers
    void bindVertexBuffers(
        const std::vector<VertexBufferId>& vertexBuffers, const std::vector<uint64_t>& offsets) const;

    /// @brief @see BaseCommandBuffer#bindIndexBuffer
    void bindIndexBuffer(IndexBufferId indexBufferId, IndexType indexType, uint64_t offset) const;

    /// @brief @see BaseCommandBuffer#bindDescriptorSet
    void bindDescriptorSet(DescriptorSetId descriptorSetId, PipelineLayoutId pipelineLayoutId, uint32_t index) const;

    /// @brief @see BaseCommandBuffer#beginDebugLabel
    void beginDebugLabel(const std::string& name, glm::vec4 color) const;

    /// @brief @see BaseCommandBuffer#endDebugLabel
    void endDebugLabel() const;

    /// @brief @see BaseCommandBuffer#insertDebugLabel
    void insertDebugLabel(const std::string& name, glm::vec4 color) const;

    /// @brief @see BaseCommandBuffer#commandBufferId
    [[nodiscard]] CommandBufferId commandBufferId() const { return _commandBuffer; }

    /// @brief @see BaseCommandBuffer#create
    /// @param name Command buffer name.
    [[nodiscard]] static CommandBufferRef create(const std::string& name);

private:
    std::string _name {}; ///< Name.
    vk::CommandBuffer _commandBuffer {}; ///< Command buffer.
};

} // namespace chronicle