#pragma once

#include "pch.h"

#include "Common.h"

#include "ImageInfo.h"
#include "PipelineInfo.h"
#include "RenderPassInfo.h"

namespace chronicle {

class App;

template <class T> class RendererI {
public:
    static void init() { T::init(); }
    static void deinit() { T::deinit(); }

    static void waitIdle() { T::waitIdle(); }
    static void waitForFence(const FenceRef& fence) { T::waitForFence(fence); }
    static void resetFence(const FenceRef& fence) { T::resetFence(fence); }
    static std::optional<uint32_t> acquireNextImage(const SemaphoreRef& semaphore)
    {
        return T::acquireNextImage(semaphore);
    }
    static void submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore, const SemaphoreRef& signalSemaphore,
        const CommandBufferRef& commandBuffer)
    {
        T::submit(fence, waitSemaphore, signalSemaphore, commandBuffer);
    }
    static bool present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex)
    {
        return T::present(waitSemaphore, imageIndex);
    }

    static void invalidateSwapChain() { T::invalidateSwapChain(); }

    [[nodiscard]] static Format swapChainFormat() { return T::swapChainFormat(); }
    [[nodiscard]] static Format depthFormat() { return T::depthFormat(); }
    [[nodiscard]] static const std::vector<ImageRef>& swapChainImages() { return T::swapChainImages(); }
    [[nodiscard]] static const ImageRef& depthImage() { return T::depthImage(); }
    [[nodiscard]] static ExtentInt2D swapChainExtent() { return T::swapChainExtent(); }

private:
    RendererI() = default;
    friend T;
};

} // namespace chronicle