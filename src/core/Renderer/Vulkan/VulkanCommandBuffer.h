// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/CommandBufferI.h"

namespace chronicle {

class VulkanCommandBuffer : public CommandBufferI<VulkanCommandBuffer>, private NonCopyable<VulkanCommandBuffer> {
protected:
    explicit VulkanCommandBuffer();

public:
    ~VulkanCommandBuffer();

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const;
    void bindPipeline(const PipelineRef& pipeline);
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const;
    void bindIndexBuffer(const IndexBufferRef& indexBuffer) const;
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const;

    [[nodiscard]] const vk::CommandBuffer& commandBuffer() const { return _commandBuffer; }

    [[nodiscard]] static CommandBufferRef create();

private:
    vk::CommandBuffer _commandBuffer;
    vk::PipelineLayout _currentPipelineLayout;
};

} // namespace chronicle