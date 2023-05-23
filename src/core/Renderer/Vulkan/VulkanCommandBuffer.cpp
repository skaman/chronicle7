// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanCommandBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanIndexBuffer.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanRenderContext.h"
#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanCommandBuffer);

VulkanCommandBuffer::VulkanCommandBuffer(const std::string& name)
    : _name(name)
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Create command buffer");

    assert(VulkanContext::commandPool);

    // create the command buffer
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setCommandPool(VulkanContext::commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(1);
    _commandBuffer = VulkanContext::device.allocateCommandBuffers(allocInfo)[0];

    assert(_commandBuffer);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::setDebugObjectName(_commandBuffer, name);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

void VulkanCommandBuffer::begin() const
{
    CHRZONE_RENDERER;

    assert(_commandBuffer);

    vk::CommandBufferBeginInfo beginInfo = {};
    _commandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::end() const
{
    CHRZONE_RENDERER;

    assert(_commandBuffer);

    _commandBuffer.end();
}

void VulkanCommandBuffer::setViewport(const ViewportInfo& viewport) const
{
    CHRZONE_RENDERER;

    assert(viewport.width > 0.0f);
    assert(viewport.height > 0.0f);
    assert(viewport.minDepth >= 0.0f && viewport.minDepth <= 1.0f);
    assert(viewport.maxDepth >= 0.0f && viewport.maxDepth <= 1.0f);
    assert(_commandBuffer);

    vk::Viewport viewportInfo = {};
    viewportInfo.setX(viewport.x);
    viewportInfo.setY(viewport.y);
    viewportInfo.setWidth(viewport.width);
    viewportInfo.setHeight(viewport.height);
    viewportInfo.setMinDepth(viewport.minDepth);
    viewportInfo.setMaxDepth(viewport.maxDepth);
    _commandBuffer.setViewport(0, viewportInfo);

    // set scissor
    _commandBuffer.setScissor(
        0, vk::Rect2D({ 0, 0 }, { static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height) }));
}

void VulkanCommandBuffer::beginRenderPass(const RenderPassBeginInfo& renderPassInfo) const
{
    CHRZONE_RENDERER;

    assert(renderPassInfo.renderPassId);
    assert(renderPassInfo.frameBufferId);
    assert(renderPassInfo.renderAreaExtent.x >= 0);
    assert(renderPassInfo.renderAreaExtent.y >= 0);
    assert(_commandBuffer);

    std::array<vk::ClearValue, 2> clearValues {};
    clearValues[0].setColor({ std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } });
    clearValues[1].setDepthStencil({ 1.0f, 0 });

    vk::RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.setRenderPass(renderPassInfo.renderPassId);
    renderPassBeginInfo.setFramebuffer(renderPassInfo.frameBufferId);
    renderPassBeginInfo.setRenderArea(
        vk::Rect2D({ renderPassInfo.renderAreaOffset.x, renderPassInfo.renderAreaOffset.y },
            { renderPassInfo.renderAreaExtent.x, renderPassInfo.renderAreaExtent.y }));
    renderPassBeginInfo.setClearValues(clearValues);
    _commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
}

void VulkanCommandBuffer::endRenderPass() const
{
    CHRZONE_RENDERER;

    assert(_commandBuffer);

    _commandBuffer.endRenderPass();
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
{
    CHRZONE_RENDERER;

    assert(indexCount > 0);
    assert(instanceCount > 0);
    assert(_commandBuffer);

    CHRLOG_TRACE("Draw indexed: index count={}, instance count={}", indexCount, instanceCount);

    // draw
    _commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void VulkanCommandBuffer::bindPipeline(PipelineId pipelineId) const
{
    CHRZONE_RENDERER;

    assert(pipelineId);
    assert(_commandBuffer);

    CHRLOG_TRACE("Bind pipeline");

    // bind the pipeline
    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineId);
}

void VulkanCommandBuffer::bindVertexBuffer(VertexBufferId vertexBufferId, uint64_t offset) const
{
    CHRZONE_RENDERER;

    assert(vertexBufferId);
    assert(_commandBuffer);

    CHRLOG_TRACE("Bind vertex buffer");

    // bind the vertex buffer
    _commandBuffer.bindVertexBuffers(0, vertexBufferId, offset);
}

void VulkanCommandBuffer::bindVertexBuffers(
    const std::vector<VertexBufferId>& vertexBuffers, const std::vector<uint64_t>& offsets) const
{
    CHRZONE_RENDERER;

    assert(!vertexBuffers.empty());
    assert(!offsets.empty());
    assert(vertexBuffers.size() == offsets.size());
    assert(_commandBuffer);

    CHRLOG_TRACE("Bind vertex buffers");

    // bind the vertex buffer
    _commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
}

void VulkanCommandBuffer::bindIndexBuffer(const IndexBufferId indexBufferId, IndexType indexType, uint64_t offset) const
{
    CHRZONE_RENDERER;

    assert(indexBufferId);
    assert(_commandBuffer);

    CHRLOG_TRACE("Bind index buffer");

    // bind the index buffer
    _commandBuffer.bindIndexBuffer(indexBufferId, offset, VulkanEnums::indexTypeToVulkan(indexType));
}

void VulkanCommandBuffer::bindDescriptorSet(
    DescriptorSetId descriptorSetId, PipelineLayoutId pipelineLayoutId, uint32_t index) const
{
    CHRZONE_RENDERER;

    assert(descriptorSetId);
    assert(pipelineLayoutId);
    assert(index >= 0 && index < 4); // max 4 descriptor sets

    CHRLOG_TRACE("Bind descriptor set: index={}", index);

    // bind the descriptor set
    _commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipelineLayoutId, index, descriptorSetId, nullptr);
}

void VulkanCommandBuffer::beginDebugLabel(const std::string& name, glm::vec4 color) const
{
#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::beginDebugLabel(_commandBuffer, name, color);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

void VulkanCommandBuffer::endDebugLabel() const
{
#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::endDebugLabel(_commandBuffer);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

void VulkanCommandBuffer::insertDebugLabel(const std::string& name, glm::vec4 color) const
{
#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::insertDebugLabel(_commandBuffer, name, color);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

CommandBufferRef VulkanCommandBuffer::create(const std::string& name)
{
    return std::make_shared<ConcreteVulkanCommandBuffer>(name);
}

} // namespace chronicle