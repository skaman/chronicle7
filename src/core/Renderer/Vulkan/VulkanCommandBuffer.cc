// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanCommandBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanIndexBuffer.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
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

void VulkanCommandBuffer::reset() const
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Reset command buffer");

    _commandBuffer.reset();
}

void VulkanCommandBuffer::begin() const
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Begin command buffer");

    vk::CommandBufferBeginInfo beginInfo = {};
    (void)_commandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::end() const
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("End command buffer");

    _commandBuffer.end();
}

void VulkanCommandBuffer::beginRenderPass(
    const RenderPassRef& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
{
    CHRZONE_RENDERER;

    assert(renderPass);

    CHRLOG_TRACE("Begin render pass: area offset={}x{}, extent={}x{}", renderArea.offset.x, renderArea.offset.y,
        renderArea.extent.width, renderArea.extent.height);

    const auto vulkanRenderPass = static_cast<VulkanRenderPass*>(renderPass.get());

    std::array<vk::ClearValue, 2> clearValues {};
    clearValues[0].setColor({ std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } });
    clearValues[1].setDepthStencil({ 1.0f, 0 });

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.setRenderPass(vulkanRenderPass->renderPass());
    renderPassInfo.setFramebuffer(vulkanRenderPass->frameBuffer(imageIndex));
    renderPassInfo.setRenderArea(vk::Rect2D(
        { renderArea.offset.x, renderArea.offset.y }, vk::Extent2D(renderArea.extent.width, renderArea.extent.height)));
    renderPassInfo.setClearValues(clearValues);

    _commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

void VulkanCommandBuffer::endRenderPass() const
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("End render pass");

    _commandBuffer.endRenderPass();
}

void VulkanCommandBuffer::setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Set viewport: area offset={}x{}, extent={}x{}, min depth={}, max depth={}", viewport.offset.x,
        viewport.offset.y, viewport.extent.width, viewport.extent.height, minDepth, maxDepth);

    vk::Viewport viewportInfo = {};
    viewportInfo.setX(viewport.offset.x);
    viewportInfo.setY(viewport.offset.y);
    viewportInfo.setWidth(viewport.extent.width);
    viewportInfo.setHeight(viewport.extent.height);
    viewportInfo.setMinDepth(minDepth);
    viewportInfo.setMaxDepth(maxDepth);

    _commandBuffer.setViewport(0, viewportInfo);
}

void VulkanCommandBuffer::setScissor(RectInt2D scissor) const
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Set scissor: area offset={}x{}, extent={}x{}", scissor.offset.x, scissor.offset.y,
        scissor.extent.width, scissor.extent.height);

    _commandBuffer.setScissor(0,
        vk::Rect2D({ scissor.offset.x, scissor.offset.y }, vk::Extent2D(scissor.extent.width, scissor.extent.height)));
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