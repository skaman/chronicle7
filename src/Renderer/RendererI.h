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
    [[nodiscard]] Format depthFormat() const { return static_cast<const T*>(this)->depthFormat(); }
    [[nodiscard]] const std::vector<ImageRef>& swapChainImages() const
    {
        return static_cast<const T*>(this)->swapChainImages();
    }
    [[nodiscard]] const ImageRef& depthImage() const { return static_cast<const T*>(this)->depthImage(); }
    [[nodiscard]] ExtentInt2D swapChainExtent() const { return static_cast<const T*>(this)->swapChainExtent(); }

    static RendererUnique create(App* app) { return T::create(app); }

private:
    RendererI() = default;
    friend T;
};

} // namespace chronicle