// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanCommandBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanIndexBuffer.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanCommandBuffer);

VulkanCommandBuffer::VulkanCommandBuffer()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create command buffer");

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setCommandPool(VulkanContext::commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(1);

    _commandBuffer = VulkanContext::device.allocateCommandBuffers(allocInfo)[0];
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy command buffer");
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
{
    CHRZONE_RENDERER;

    assert(indexCount > 0);
    assert(instanceCount > 0);

    CHRLOG_TRACE("Draw indexed: index count={}, instance count={}", indexCount, instanceCount);

    _commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void VulkanCommandBuffer::bindPipeline(const PipelineRef& pipeline)
{
    CHRZONE_RENDERER;

    assert(pipeline);

    CHRLOG_TRACE("Bind pipeline");

    const auto vulkanPipeline = static_cast<VulkanPipeline*>(pipeline.get());

    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline->pipeline());
    _currentPipelineLayout = vulkanPipeline->pipelineLayout();
}

void VulkanCommandBuffer::bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
{
    CHRZONE_RENDERER;

    assert(vertexBuffer);

    CHRLOG_TRACE("Bind vertex buffer");

    const auto vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());

    _commandBuffer.bindVertexBuffers(0, vulkanVertexBuffer->buffer(), vk::DeviceSize(0));
}

void VulkanCommandBuffer::bindIndexBuffer(const IndexBufferRef& indexBuffer) const
{
    CHRZONE_RENDERER;

    assert(indexBuffer);

    CHRLOG_TRACE("Bind index buffer");

    const auto vulkanIndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());

    _commandBuffer.bindIndexBuffer(vulkanIndexBuffer->buffer(), vk::DeviceSize(0), vk::IndexType::eUint32);
}

void VulkanCommandBuffer::bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
{
    CHRZONE_RENDERER;

    assert(descriptorSet);

    CHRLOG_TRACE("Bind descriptor set: index={}", index);

    const auto vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptorSet.get());

    _commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _currentPipelineLayout, index, vulkanDescriptorSet->descriptorSet(), nullptr);
}

CommandBufferRef VulkanCommandBuffer::create() { return std::make_shared<ConcreteVulkanCommandBuffer>(); }

} // namespace chronicle