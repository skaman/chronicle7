#include "VulkanRenderer.h"

#include "Platform/App.h"

#include "VulkanCommandBuffer.h"
#include "VulkanFence.h"
#include "VulkanImage.h"
#include "VulkanInstance.h"
#include "VulkanSemaphore.h"

namespace chronicle {

void VulkanRenderer::init(App* app) { VulkanInstance::init(app); }

void VulkanRenderer::deinit() { VulkanInstance::deinit(); }

void VulkanRenderer::waitIdle()
{
    CHRZONE_VULKAN

    VulkanContext::device.waitIdle();
}

void VulkanRenderer::waitForFence(const FenceRef& fence)
{
    CHRZONE_VULKAN

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());
    (void)VulkanContext::device.waitForFences(vulkanFence->fence(), true, std::numeric_limits<uint64_t>::max());
}

void VulkanRenderer::resetFence(const FenceRef& fence)
{
    CHRZONE_VULKAN

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());
    (void)VulkanContext::device.resetFences(vulkanFence->fence());
}

std::optional<uint32_t> VulkanRenderer::acquireNextImage(const SemaphoreRef& semaphore)
{
    CHRZONE_VULKAN

    const auto vulkanSemaphore = static_cast<const VulkanSemaphore*>(semaphore.get());

    try {
        auto result = VulkanContext::device.acquireNextImageKHR(
            VulkanContext::swapChain, std::numeric_limits<uint64_t>::max(), vulkanSemaphore->semaphore(), nullptr);
        return result.value;
    } catch (vk::OutOfDateKHRError err) {
        VulkanInstance::recreateSwapChain();
        return std::optional<uint32_t>();
    }
}

void VulkanRenderer::submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore,
    const SemaphoreRef& signalSemaphore, const CommandBufferRef& commandBuffer)
{
    CHRZONE_VULKAN

    const auto vulkanWaitSemaphore = static_cast<const VulkanSemaphore*>(waitSemaphore.get());
    const auto vulkanSignalSemaphore = static_cast<const VulkanSemaphore*>(signalSemaphore.get());

    const auto vulkanCommandBuffer = static_cast<const VulkanCommandBuffer*>(commandBuffer.get());

    vk::SubmitInfo submitInfo = {};
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(vulkanWaitSemaphore->semaphore());
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(vulkanCommandBuffer->commandBuffer());
    submitInfo.setSignalSemaphores(vulkanSignalSemaphore->semaphore());

    const auto vulkanFence = static_cast<const VulkanFence*>(fence.get());

    VulkanContext::graphicsQueue.submit(submitInfo, vulkanFence->fence());
}

bool VulkanRenderer::present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex)
{
    CHRZONE_VULKAN

    const auto vulkanWaitSemaphore = static_cast<const VulkanSemaphore*>(waitSemaphore.get());

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(vulkanWaitSemaphore->semaphore());
    presentInfo.setSwapchains(VulkanContext::swapChain);
    presentInfo.setImageIndices(imageIndex);

    vk::Result resultPresent;
    try {
        resultPresent = VulkanContext::presentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError err) {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    }

    if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR
        || VulkanContext::swapChainInvalidated) {
        VulkanContext::swapChainInvalidated = false;
        VulkanInstance::recreateSwapChain();
        return false;
    }

    return true;
}

} // namespace chronicle