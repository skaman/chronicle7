// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanRenderer.h"

#include "VulkanCommandBuffer.h"
#include "VulkanInstance.h"

namespace chronicle {

void VulkanRenderer::init()
{
    CHRLOG_INFO("Renderer init");

    VulkanInstance::init();
}

void VulkanRenderer::deinit()
{
    CHRZONE_RENDERER;

    CHRLOG_INFO("Renderer deinit");

    VulkanInstance::deinit();
}

void VulkanRenderer::waitIdle()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Wait idle");

    VulkanContext::device.waitIdle();
}

bool VulkanRenderer::beginFrame()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Begin swapchain");

    (void)VulkanContext::device.waitForFences(
        VulkanContext::inFlightFences[VulkanContext::currentFrame], true, std::numeric_limits<uint64_t>::max());

    VulkanContext::device.resetFences(VulkanContext::inFlightFences[VulkanContext::currentFrame]);
    try {
        auto result
            = VulkanContext::device.acquireNextImageKHR(VulkanContext::swapChain, std::numeric_limits<uint64_t>::max(),
                VulkanContext::imageAvailableSemaphores[VulkanContext::currentFrame], nullptr);
        VulkanContext::currentImage = result.value;
    } catch (vk::OutOfDateKHRError err) {
        VulkanInstance::recreateSwapChain();
        return false;
    }
    // chronicle::Gui::newFrame();

    const auto& vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer().get());

    CHRLOG_TRACE(
        "New frame: area extent={}x{}", VulkanContext::swapChainExtent.width, VulkanContext::swapChainExtent.height);

    vk::CommandBufferBeginInfo beginInfo = {};
    vulkanCommandBuffer->commandBuffer().begin(beginInfo);

    std::array<vk::ClearValue, 2> clearValues {};
    clearValues[0].setColor({ std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } });
    clearValues[1].setDepthStencil({ 1.0f, 0 });

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.setRenderPass(VulkanContext::renderPass);
    renderPassInfo.setFramebuffer(VulkanContext::framebuffers[VulkanContext::currentImage]);
    renderPassInfo.setRenderArea(vk::Rect2D({ 0, 0 }, VulkanContext::swapChainExtent));
    renderPassInfo.setClearValues(clearValues);

    vulkanCommandBuffer->commandBuffer().beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewportInfo = {};
    viewportInfo.setX(0.0f);
    viewportInfo.setY(0.0f);
    viewportInfo.setWidth(static_cast<float>(VulkanContext::swapChainExtent.width));
    viewportInfo.setHeight(static_cast<float>(VulkanContext::swapChainExtent.height));
    viewportInfo.setMinDepth(0.0f);
    viewportInfo.setMaxDepth(1.0f);

    vulkanCommandBuffer->commandBuffer().setViewport(0, viewportInfo);
    vulkanCommandBuffer->commandBuffer().setScissor(0, vk::Rect2D({ 0, 0 }, VulkanContext::swapChainExtent));

    return true;
}

void VulkanRenderer::endFrame()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("End swapchain");

    // chronicle::Gui::render(_commandBuffers[_currentFrame]);
    const auto& vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer().get());

    vulkanCommandBuffer->commandBuffer().endRenderPass();
    vulkanCommandBuffer->commandBuffer().end();

    vk::SubmitInfo submitInfo = {};
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(VulkanContext::imageAvailableSemaphores[VulkanContext::currentFrame]);
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(vulkanCommandBuffer->commandBuffer());
    submitInfo.setSignalSemaphores(VulkanContext::renderFinishedSemaphores[VulkanContext::currentFrame]);
    VulkanContext::graphicsQueue.submit(submitInfo, VulkanContext::inFlightFences[VulkanContext::currentFrame]);

    uint32_t imageIndex = VulkanContext::currentImage;
    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(VulkanContext::renderFinishedSemaphores[VulkanContext::currentFrame]);
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
    }

    VulkanContext::currentFrame = (VulkanContext::currentFrame + 1) % VulkanContext::maxFramesInFlight;

    FrameMark;
}

} // namespace chronicle