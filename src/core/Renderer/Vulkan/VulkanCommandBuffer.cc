#include "VulkanCommandBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanIndexBuffer.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanCommandBuffer)

VulkanCommandBuffer::VulkanCommandBuffer()
{
    CHRZONE_RENDERER

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setCommandPool(VulkanContext::commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(1);

    _commandBuffer = VulkanContext::device.allocateCommandBuffers(allocInfo)[0];
}

void VulkanCommandBuffer::reset() const
{
    CHRZONE_RENDERER

    _commandBuffer.reset();
}

void VulkanCommandBuffer::begin() const
{
    CHRZONE_RENDERER

    vk::CommandBufferBeginInfo beginInfo = {};
    (void)_commandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::end() const { _commandBuffer.end(); }

void VulkanCommandBuffer::beginRenderPass(
    const RenderPassRef& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
{
    CHRZONE_RENDERER

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
    CHRZONE_RENDERER

    _commandBuffer.endRenderPass();
}

void VulkanCommandBuffer::setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
{
    CHRZONE_RENDERER

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
    CHRZONE_RENDERER

    _commandBuffer.setScissor(0,
        vk::Rect2D({ scissor.offset.x, scissor.offset.y }, vk::Extent2D(scissor.extent.width, scissor.extent.height)));
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
{
    CHRZONE_RENDERER

    _commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void VulkanCommandBuffer::bindPipeline(const PipelineRef& pipeline)
{
    CHRZONE_RENDERER

    const auto vulkanPipeline = static_cast<VulkanPipeline*>(pipeline.get());

    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline->pipeline());
    _currentPipelineLayout = vulkanPipeline->pipelineLayout();
}

void VulkanCommandBuffer::bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
{
    CHRZONE_RENDERER

    const auto vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());

    _commandBuffer.bindVertexBuffers(0, vulkanVertexBuffer->buffer(), vk::DeviceSize(0));
}

void VulkanCommandBuffer::bindIndexBuffer(const IndexBufferRef& indexBuffer) const
{
    CHRZONE_RENDERER

    const auto vulkanIndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());

    _commandBuffer.bindIndexBuffer(vulkanIndexBuffer->buffer(), vk::DeviceSize(0), vk::IndexType::eUint32);
}

void VulkanCommandBuffer::bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
{
    CHRZONE_RENDERER

    const auto vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptorSet.get());

    _commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _currentPipelineLayout, index, vulkanDescriptorSet->descriptorSet(), nullptr);
}

CommandBufferRef VulkanCommandBuffer::create() { return std::make_shared<ConcreteVulkanCommandBuffer>(); }

} // namespace chronicle