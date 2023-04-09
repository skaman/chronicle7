#pragma once

#include "pch.h"

#include "Common.h"

#include "ImageInfo.h"
#include "PipelineInfo.h"

// #include "CommandBuffer.h"
// #include "DescriptorSet.h"
// #include "Fence.h"
// #include "Image.h"
// #include "IndexBuffer.h"
// #include "Pipeline.h"
// #include "RenderPass.h"
// #include "RenderPassInfo.h"
// #include "Semaphore.h"
// #include "VertexBuffer.h"

// #include "Vulkan/VulkanCommandBuffer.h"
// #include "Vulkan/VulkanFence.h"
// #include "Vulkan/VulkanImage.h"
// #include "Vulkan/VulkanIndexBuffer.h"
// #include "Vulkan/VulkanPipeline.h"
// #include "Vulkan/VulkanRenderPass.h"
// #include "Vulkan/VulkanSemaphore.h"
// #include "Vulkan/VulkanVertexBuffer.h"

namespace chronicle {

class App;

template <class T> class RendererI {
public:
    void waitIdle() const { static_cast<const T*>(this)->waitIdle(); }

    void waitForFence(const FenceRef& fence) const { static_cast<const T*>(this)->waitForFence(fence); }
    void resetFence(const FenceRef& fence) const { static_cast<const T*>(this)->resetFence(fence); }
    uint32_t acquireNextImage(const SemaphoreRef& semaphore)
    {
        return static_cast<T*>(this)->acquireNextImage(semaphore);
    }
    void submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore, const SemaphoreRef& signalSemaphore,
        const CommandBufferRef& commandBuffer) const
    {
        static_cast<const T*>(this)->submit(fence, waitSemaphore, signalSemaphore, commandBuffer);
    }
    bool present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex)
    {
        return static_cast<T*>(this)->present(waitSemaphore, imageIndex);
    }

    void invalidateSwapChain() { static_cast<T*>(this)->invalidateSwapChain(); }

    [[nodiscard]] Format swapChainFormat() const { return static_cast<const T*>(this)->swapChainFormat(); }
    [[nodiscard]] const std::vector<ImageRef>& swapChainImages() const
    {
        return static_cast<const T*>(this)->swapChainImages();
    }
    [[nodiscard]] ExtentInt2D swapChainExtent() const { return static_cast<const T*>(this)->swapChainExtent(); }

    [[nodiscard]] RenderPassRef createRenderPass(const RenderPassInfo& renderPassInfo) const
    {
        return RenderPass::create(this, renderPassInfo);
    }
    [[nodiscard]] DescriptorSetRef createDescriptorSet() const { return DescriptorSet::create(this); }
    [[nodiscard]] PipelineRef createPipeline(const PipelineInfo& pipelineInfo) const
    {
        return Pipeline::create(this, pipelineInfo);
    }
    [[nodiscard]] VertexBufferRef createVertexBuffer() const { return VertexBuffer::create(this); }
    [[nodiscard]] IndexBufferRef createIndexBuffer() const { return IndexBuffer::create(this); }
    [[nodiscard]] CommandBufferRef createCommandBuffer() const { return CommandBuffer::create(this); }
    [[nodiscard]] SemaphoreRef createSemaphore() const { return Semaphore::create(this); }
    [[nodiscard]] FenceRef createFence() const { return Fence::create(this); }
    [[nodiscard]] ImageRef createImage(const ImageInfo& imageInfo) const { return Image::create(this, imageInfo); }

    static RendererRef create(App* app) { return T::create(app); }

private:
    RendererI() = default;
    friend T;
};

} // namespace chronicle