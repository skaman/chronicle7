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
    explicit VulkanCommandBuffer(const char* debugName);

public:
    /// @brief Destructor.
    ~VulkanCommandBuffer() = default;

    /// @brief @see CommandBufferI#drawIndexed
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const;

    /// @brief @see CommandBufferI#bindPipeline
    void bindPipeline(const PipelineRef& pipeline);

    /// @brief @see CommandBufferI#bindVertexBuffer
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const;

    /// @brief @see CommandBufferI#bindIndexBuffer
    void bindIndexBuffer(const IndexBufferRef& indexBuffer) const;

    /// @brief @see CommandBufferI#bindDescriptorSet
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const;

    /// @brief @see CommandBufferI#beginDebugLabel
    void beginDebugLabel(const char* name, glm::vec4 color) const;

    /// @brief @see CommandBufferI#endDebugLabel
    void endDebugLabel() const;

    /// @brief @see CommandBufferI#insertDebugLabel
    void insertDebugLabel(const char* name, glm::vec4 color) const;

    /// @brief Get the vulkan handle for the command buffer.
    /// @return Vulkan handle.
    [[nodiscard]] const vk::CommandBuffer& commandBuffer() const { return _commandBuffer; }

    /// @brief @see CommandBufferI#create
    /// @param debugName Debug name.
    [[nodiscard]] static CommandBufferRef create(const char* debugName);

private:
    vk::CommandBuffer _commandBuffer; ///< Command buffer.
    vk::PipelineLayout _currentPipelineLayout; ///< Current pipeline layout.
};

} // namespace chronicle