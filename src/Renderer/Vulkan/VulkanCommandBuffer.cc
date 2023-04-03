#include "VulkanCommandBuffer.h"

#include "Renderer/DescriptorSet.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderPass.h"
#include "Renderer/VertexBuffer.h"

namespace chronicle {

VulkanCommandBuffer::VulkanCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool)
    : _device(device)
    , _commandPool(commandPool)
{
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setCommandPool(_commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(1);

    _commandBuffer = _device.allocateCommandBuffers(allocInfo)[0];
}

void VulkanCommandBuffer::reset() const { _commandBuffer.reset(); }

void VulkanCommandBuffer::begin() const
{
    vk::CommandBufferBeginInfo beginInfo = {};
    (void)_commandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::end() const { _commandBuffer.end(); }

void VulkanCommandBuffer::beginRenderPass(
    const std::shared_ptr<RenderPass>& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
{
    vk::ClearValue clearColor = { std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } };
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.setRenderPass(renderPass->native().renderPass());
    renderPassInfo.setFramebuffer(renderPass->native().frameBuffer(imageIndex));
    renderPassInfo.setRenderArea(vk::Rect2D(
        { renderArea.offset.x, renderArea.offset.y }, vk::Extent2D(renderArea.extent.width, renderArea.extent.height)));
    renderPassInfo.setClearValues(clearColor);

    _commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

void VulkanCommandBuffer::endRenderPass() const { _commandBuffer.endRenderPass(); }

void VulkanCommandBuffer::setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
{
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
    _commandBuffer.setScissor(0,
        vk::Rect2D({ scissor.offset.x, scissor.offset.y }, vk::Extent2D(scissor.extent.width, scissor.extent.height)));
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
{
    _commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void VulkanCommandBuffer::bindPipeline(const std::shared_ptr<Pipeline>& pipeline)
{
    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->native().pipeline());
    _currentPipelineLayout = pipeline->native().pipelineLayout();
}

void VulkanCommandBuffer::bindVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) const
{
    _commandBuffer.bindVertexBuffers(0, vertexBuffer->native().buffer(), vk::DeviceSize(0));
}

void VulkanCommandBuffer::bindIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) const
{
    _commandBuffer.bindIndexBuffer(indexBuffer->native().buffer(), vk::DeviceSize(0), vk::IndexType::eUint16);
}

void VulkanCommandBuffer::bindDescriptorSet(const std::shared_ptr<DescriptorSet>& descriptorSet, uint32_t index) const
{
    _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _currentPipelineLayout, index,
        descriptorSet->native().descriptorSet(), nullptr);
}

} // namespace chronicle