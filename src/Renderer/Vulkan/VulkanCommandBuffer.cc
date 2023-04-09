#include "VulkanCommandBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanIndexBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderer.h"
#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanCommandBuffer)

VulkanCommandBuffer::VulkanCommandBuffer(const vk::Device& device, const vk::CommandPool& commandPool)
    : _device(device)
    , _commandPool(commandPool)
{
    CHRZONE_VULKAN

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setCommandPool(_commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(1);

    _commandBuffer = _device.allocateCommandBuffers(allocInfo)[0];
}

void VulkanCommandBuffer::reset() const
{
    CHRZONE_VULKAN

    _commandBuffer.reset();
}

void VulkanCommandBuffer::begin() const
{
    CHRZONE_VULKAN

    vk::CommandBufferBeginInfo beginInfo = {};
    (void)_commandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::end() const { _commandBuffer.end(); }

void VulkanCommandBuffer::beginRenderPass(
    const RenderPassRef& renderPass, const RectInt2D& renderArea, uint32_t imageIndex) const
{
    CHRZONE_VULKAN

    const auto vulkanRenderPass = static_cast<VulkanRenderPass*>(renderPass.get());

    vk::ClearValue clearColor = { std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } };
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.setRenderPass(vulkanRenderPass->renderPass());
    renderPassInfo.setFramebuffer(vulkanRenderPass->frameBuffer(imageIndex));
    renderPassInfo.setRenderArea(vk::Rect2D(
        { renderArea.offset.x, renderArea.offset.y }, vk::Extent2D(renderArea.extent.width, renderArea.extent.height)));
    renderPassInfo.setClearValues(clearColor);

    _commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

void VulkanCommandBuffer::endRenderPass() const
{
    CHRZONE_VULKAN

    _commandBuffer.endRenderPass();
}

void VulkanCommandBuffer::setViewport(RectFloat2D viewport, float minDepth, float maxDepth) const
{
    CHRZONE_VULKAN

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
    CHRZONE_VULKAN

    _commandBuffer.setScissor(0,
        vk::Rect2D({ scissor.offset.x, scissor.offset.y }, vk::Extent2D(scissor.extent.width, scissor.extent.height)));
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
{
    CHRZONE_VULKAN

    _commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void VulkanCommandBuffer::bindPipeline(const PipelineRef& pipeline)
{
    CHRZONE_VULKAN

    const auto vulkanPipeline = static_cast<VulkanPipeline*>(pipeline.get());

    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline->pipeline());
    _currentPipelineLayout = vulkanPipeline->pipelineLayout();
}

void VulkanCommandBuffer::bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
{
    CHRZONE_VULKAN

    const auto vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());

    _commandBuffer.bindVertexBuffers(0, vulkanVertexBuffer->buffer(), vk::DeviceSize(0));
}

void VulkanCommandBuffer::bindIndexBuffer(const IndexBufferRef& indexBuffer) const
{
    CHRZONE_VULKAN

    const auto vulkanIndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());

    _commandBuffer.bindIndexBuffer(vulkanIndexBuffer->buffer(), vk::DeviceSize(0), vk::IndexType::eUint16);
}

void VulkanCommandBuffer::bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
{
    CHRZONE_VULKAN

    const auto vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptorSet.get());

    _commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _currentPipelineLayout, index, vulkanDescriptorSet->descriptorSet(), nullptr);
}

CommandBufferRef VulkanCommandBuffer::create(const Renderer* renderer)
{
    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);

    return std::make_shared<ConcreteVulkanCommandBuffer>(vulkanRenderer->device(), vulkanRenderer->commandPool());
}

} // namespace chronicle