// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanCommandBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanIndexBuffer.h"
#include "VulkanInstance.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanPipeline.h"
#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanCommandBuffer);

VulkanCommandBuffer::VulkanCommandBuffer(const char* debugName)
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create command buffer");

    // create the command buffer
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setCommandPool(VulkanContext::commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandBufferCount(1);
    _commandBuffer = VulkanContext::device.allocateCommandBuffers(allocInfo)[0];

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    // set the debug object name
    VulkanUtils::setDebugObjectName(_commandBuffer, debugName);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
{
    CHRZONE_RENDERER;

    assert(indexCount > 0);
    assert(instanceCount > 0);

    CHRLOG_TRACE("Draw indexed: index count={}, instance count={}", indexCount, instanceCount);

    // draw
    _commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

void VulkanCommandBuffer::bindPipeline(const PipelineRef& pipeline)
{
    CHRZONE_RENDERER;

    assert(pipeline);

    CHRLOG_TRACE("Bind pipeline");

    // cast the pipeline to vulkan pipeline
    const auto vulkanPipeline = static_cast<VulkanPipeline*>(pipeline.get());

    // bind the pipeline
    _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline->pipeline());

    // keep track of the current pipeline layout
    _currentPipelineLayout = vulkanPipeline->pipelineLayout();
}

void VulkanCommandBuffer::bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
{
    CHRZONE_RENDERER;

    assert(vertexBuffer);

    CHRLOG_TRACE("Bind vertex buffer");

    // cast the vertex buffer to vulkan vertex buffer
    const auto vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(vertexBuffer.get());

    // bind the vertex buffer
    _commandBuffer.bindVertexBuffers(0, vulkanVertexBuffer->buffer(), vk::DeviceSize(0));
}

void VulkanCommandBuffer::bindIndexBuffer(const IndexBufferRef& indexBuffer) const
{
    CHRZONE_RENDERER;

    assert(indexBuffer);

    CHRLOG_TRACE("Bind index buffer");

    // cast the index buffer to vulkan index buffer
    const auto vulkanIndexBuffer = static_cast<VulkanIndexBuffer*>(indexBuffer.get());

    // bind the index buffer
    _commandBuffer.bindIndexBuffer(vulkanIndexBuffer->buffer(), vk::DeviceSize(0), vk::IndexType::eUint32);
}

void VulkanCommandBuffer::bindMesh(const MeshRef& mesh, uint32_t submeshIndex) const
{
    CHRZONE_RENDERER;

    assert(mesh);
    assert(mesh->submeshCount() > submeshIndex);

    CHRLOG_TRACE("Bind mesh");

    // cast the mesh to vulkan mesh
    const auto vulkanMesh = static_cast<VulkanMesh*>(mesh.get());

    // get the binding informations
    const auto& bindingInfo = vulkanMesh->bindingInfo(submeshIndex);

    // bind the vertex buffers
    _commandBuffer.bindVertexBuffers(0, bindingInfo.vertexBuffers, bindingInfo.vertexBuffersOffsets);

    // bind the index buffer
    _commandBuffer.bindIndexBuffer(bindingInfo.indexBuffer, bindingInfo.indexBufferOffset, bindingInfo.indexType);

    // bind the descriptor set
    //bindDescriptorSet(vulkanMesh->material(submeshIndex)->descriptorSet(), 1);
}

void VulkanCommandBuffer::bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
{
    CHRZONE_RENDERER;

    assert(descriptorSet);

    CHRLOG_TRACE("Bind descriptor set: index={}", index);

    // cast the descriptor set to vulkan descriptor set
    const auto vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptorSet.get());

    // bind the descriptor set
    _commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _currentPipelineLayout, index, vulkanDescriptorSet->descriptorSet(), nullptr);
}

void VulkanCommandBuffer::beginDebugLabel(const char* name, glm::vec4 color) const
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

void VulkanCommandBuffer::insertDebugLabel(const char* name, glm::vec4 color) const
{
#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::insertDebugLabel(_commandBuffer, name, color);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

CommandBufferRef VulkanCommandBuffer::create(const char* debugName)
{
    return std::make_shared<ConcreteVulkanCommandBuffer>(debugName);
}

} // namespace chronicle