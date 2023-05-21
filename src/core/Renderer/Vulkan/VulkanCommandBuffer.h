// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/CommandBufferI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref CommandBufferI
class VulkanCommandBuffer : public CommandBufferI<VulkanCommandBuffer>, private NonCopyable<VulkanCommandBuffer> {
protected:
    /// @brief Default constructor.
    /// @param debugName Debug name.
    explicit VulkanCommandBuffer(const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanCommandBuffer() = default;

    /// @brief @see CommandBufferI#name
    [[nodiscard]] std::string name() const { return _name; }

    /// @brief @see CommandBufferI#begin
    void begin() const;

    /// @brief @see CommandBufferI#end
    void end() const;

    /// @brief @see CommandBufferI#setViewport
    void setViewport(const Viewport& viewport) const;

    /// @brief @see CommandBufferI#beginRenderPass
    void beginRenderPass(const RenderPassBeginInfo& renderPassInfo) const;

    /// @brief @see CommandBufferI#endRenderPass
    void endRenderPass() const;

    /// @brief @see CommandBufferI#drawIndexed
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const;

    /// @brief @see CommandBufferI#bindPipeline
    void bindPipeline(PipelineId pipelineId) const;

    /// @brief @see CommandBufferI#bindVertexBuffer
    void bindVertexBuffer(VertexBufferId vertexBufferId, uint64_t offset) const;

    /// @brief @see CommandBufferI#bindVertexBuffers
    void bindVertexBuffers(
        const std::vector<VertexBufferId>& vertexBuffers, const std::vector<uint64_t>& offsets) const;

    /// @brief @see CommandBufferI#bindIndexBuffer
    void bindIndexBuffer(IndexBufferId indexBufferId, IndexType indexType, uint64_t offset) const;

    /// @brief @see CommandBufferI#bindDescriptorSet
    void bindDescriptorSet(DescriptorSetId descriptorSetId, PipelineLayoutId pipelineLayoutId, uint32_t index) const;

    /// @brief @see CommandBufferI#beginDebugLabel
    void beginDebugLabel(const std::string& name, glm::vec4 color) const;

    /// @brief @see CommandBufferI#endDebugLabel
    void endDebugLabel() const;

    /// @brief @see CommandBufferI#insertDebugLabel
    void insertDebugLabel(const std::string& name, glm::vec4 color) const;

    /// @brief @see CommandBufferI#commandBufferId
    [[nodiscard]] CommandBufferId commandBufferId() const { return _commandBuffer; }

    /// @brief @see CommandBufferI#create
    /// @param name Command buffer name.
    [[nodiscard]] static CommandBufferRef create(const std::string& name);

private:
    std::string _name; ///< Name.
    vk::CommandBuffer _commandBuffer; ///< Command buffer.
};

} // namespace chronicle