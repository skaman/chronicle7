#pragma once

#include "pch.h"

#include "Renderer/RendererI.h"
#include "VulkanCommon.h"

namespace chronicle {

class VulkanRenderer : public RendererI<VulkanRenderer>, private NonCopyable<VulkanRenderer> {
public:
    static void init(App* app);
    static void deinit();

    static void waitIdle();
    static void waitForFence(const FenceRef& fence);
    static void resetFence(const FenceRef& fence);
    static std::optional<uint32_t> acquireNextImage(const SemaphoreRef& semaphore);
    static void submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore, const SemaphoreRef& signalSemaphore,
        const CommandBufferRef& commandBuffer);
    static bool present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex);

    static void invalidateSwapChain() { VulkanContext::swapChainInvalidated = true; }

    [[nodiscard]] static Format swapChainFormat() { return formatFromVulkan(VulkanContext::swapChainImageFormat); }
    [[nodiscard]] static const std::vector<ImageRef>& swapChainImages() { return VulkanContext::swapChainImages; }
    [[nodiscard]] static ExtentInt2D swapChainExtent()
    {
        return ExtentInt2D(VulkanContext::swapChainExtent.width, VulkanContext::swapChainExtent.height);
    }
    [[nodiscard]] static Format depthFormat() { return formatFromVulkan(VulkanContext::depthImageFormat); }
    [[nodiscard]] static const ImageRef& depthImage() { return VulkanContext::depthImage; }
};

} // namespace chronicle