#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanRenderer.h"
#endif

#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "Image.h"
#include "IndexBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "RenderPassInfo.h"
#include "Semaphore.h"
#include "VertexBuffer.h"

namespace chronicle {

class App;

class Renderer {
public:
    explicit Renderer(chronicle::App* app)
        : _renderer(app) {};

    inline void waitIdle() const { _renderer.waitIdle(); }

    inline void waitForFence(const std::shared_ptr<Fence>& fence) const { _renderer.waitForFence(fence); }
    inline void resetFence(const std::shared_ptr<Fence>& fence) const { _renderer.resetFence(fence); }
    inline uint32_t acquireNextImage(const std::shared_ptr<Semaphore>& semaphore)
    {
        return _renderer.acquireNextImage(semaphore);
    }
    inline void submit(const std::shared_ptr<Fence>& fence, const std::shared_ptr<Semaphore>& waitSemaphore,
        const std::shared_ptr<Semaphore>& signalSemaphore, const std::shared_ptr<CommandBuffer>& commandBuffer) const
    {
        _renderer.submit(fence, waitSemaphore, signalSemaphore, commandBuffer);
    }
    inline bool present(const std::shared_ptr<Semaphore>& waitSemaphore, uint32_t imageIndex)
    {
        return _renderer.present(waitSemaphore, imageIndex);
    }

    inline void invalidateSwapChain() { _renderer.invalidateSwapChain(); }

    [[nodiscard]] inline Format swapChainFormat() const { return _renderer.swapChainFormat(); }
    [[nodiscard]] inline const std::vector<std::shared_ptr<Image>>& swapChainImages() const
    {
        return _renderer.swapChainImages();
    }
    [[nodiscard]] inline ExtentInt2D swapChainExtent() const { return _renderer.swapChainExtent(); }

    [[nodiscard]] inline std::shared_ptr<RenderPass> createRenderPass(const RenderPassInfo& renderPassInfo) const
    {
        return std::make_shared<RenderPass>(this, renderPassInfo);
    }
    [[nodiscard]] inline std::shared_ptr<DescriptorSet> createDescriptorSet() const
    {
        return std::make_shared<DescriptorSet>(this);
    }
    [[nodiscard]] inline std::shared_ptr<Pipeline> createPipeline(const PipelineInfo& pipelineInfo) const
    {
        return std::make_shared<Pipeline>(this, pipelineInfo);
    }
    [[nodiscard]] inline std::shared_ptr<VertexBuffer> createVertexBuffer() const
    {
        return std::make_shared<VertexBuffer>(this);
    }
    [[nodiscard]] inline std::shared_ptr<IndexBuffer> createIndexBuffer() const
    {
        return std::make_shared<IndexBuffer>(this);
    }
    [[nodiscard]] inline std::shared_ptr<CommandBuffer> createCommandBuffer() const
    {
        return std::make_shared<CommandBuffer>(this);
    }
    [[nodiscard]] inline std::shared_ptr<Semaphore> createSemaphore() const
    {
        return std::make_shared<Semaphore>(this);
    }
    [[nodiscard]] inline std::shared_ptr<Fence> createFence() const { return std::make_shared<Fence>(this); }
    [[nodiscard]] inline std::shared_ptr<Image> createImage(const ImageInfo& imageInfo) const
    {
        return std::make_shared<Image>(this, imageInfo);
    }

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanRenderer& native() const { return _renderer; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanRenderer _renderer;
#endif
};

} // namespace chronicle