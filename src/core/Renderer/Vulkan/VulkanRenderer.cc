// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanRenderer.h"

#include "VulkanCommandBuffer.h"
#include "VulkanEvents.h"
#include "VulkanImGui.h"
#include "VulkanInstance.h"
#include "VulkanRenderPass.h"
#include "VulkanUtils.h"

namespace chronicle {

void VulkanRenderer::init()
{
    CHRLOG_INFO("Renderer init");

    // initalize vulkan instance
    VulkanInstance::init();

    // initialize imgui
    VulkanImGui::init();
}

void VulkanRenderer::deinit()
{
    CHRZONE_RENDERER;

    CHRLOG_INFO("Renderer deinit");

    // deinitialize imgui
    VulkanImGui::deinit();

    // deinitialize vulkan instance
    VulkanInstance::deinit();
}

void VulkanRenderer::waitIdle()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Wait idle");

    // wait for GPU idle
    VulkanContext::device.waitIdle();
}

bool VulkanRenderer::beginFrame()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Begin swapchain");

    // get the current frame data
    VulkanFrameData& frameData = VulkanContext::framesData[VulkanContext::currentFrame];

    // wait for fence (image GPU processing completed)
    (void)VulkanContext::device.waitForFences(frameData.inFlightFence, true, std::numeric_limits<uint64_t>::max());

    // reset the fence
    VulkanContext::device.resetFences(frameData.inFlightFence);

    // clean the frame garbage collector
    VulkanUtils::cleanupGarbageCollector(frameData.garbageCollector);

    // acquire the image
    try {
        auto result = VulkanContext::device.acquireNextImageKHR(
            VulkanContext::swapChain, std::numeric_limits<uint64_t>::max(), frameData.imageAvailableSemaphore, nullptr);
        VulkanContext::currentImage = result.value;
    } catch (const vk::OutOfDateKHRError&) {
        VulkanInstance::recreateSwapChain();
        return false;
    }

    // get the current image data
    const VulkanImageData& imageData = VulkanContext::imagesData[VulkanContext::currentImage];

    // new imgui frame
    VulkanImGui::newFrame();

    // cast the command buffer to a vulkan command buffer
    const auto& vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer().get());

    CHRLOG_TRACE(
        "New frame: area extent={}x{}", VulkanContext::swapChainExtent.width, VulkanContext::swapChainExtent.height);

    // begin main command buffer
    vk::CommandBufferBeginInfo beginInfo = {};
    vulkanCommandBuffer->commandBuffer().begin(beginInfo);

    // begin draw pass
    std::array<vk::ClearValue, 2> clearValues {};
    clearValues[0].setColor({ std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } });
    clearValues[1].setDepthStencil({ 1.0f, 0 });

    // begin render pass
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.setRenderPass(*static_cast<const vk::RenderPass*>(VulkanContext::renderPass->renderPassId()));
    renderPassInfo.setFramebuffer(imageData.framebuffer);
    renderPassInfo.setRenderArea(vk::Rect2D({ 0, 0 }, VulkanContext::swapChainExtent));
    renderPassInfo.setClearValues(clearValues);
    vulkanCommandBuffer->commandBuffer().beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    // set viewport
    vk::Viewport viewportInfo = {};
    viewportInfo.setX(0.0f);
    viewportInfo.setY(0.0f);
    viewportInfo.setWidth(static_cast<float>(VulkanContext::swapChainExtent.width));
    viewportInfo.setHeight(static_cast<float>(VulkanContext::swapChainExtent.height));
    viewportInfo.setMinDepth(0.0f);
    viewportInfo.setMaxDepth(1.0f);
    vulkanCommandBuffer->commandBuffer().setViewport(0, viewportInfo);

    // set scissor
    vulkanCommandBuffer->commandBuffer().setScissor(0, vk::Rect2D({ 0, 0 }, VulkanContext::swapChainExtent));

    return true;
}

void VulkanRenderer::endFrame()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("End swapchain");

    // get the current frame data
    const VulkanFrameData& frameData = VulkanContext::framesData[VulkanContext::currentFrame];

    // get the current image data
    const VulkanImageData& imageData = VulkanContext::imagesData[VulkanContext::currentImage];

    // cast the command buffer to a vulkan command buffer
    const auto& vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer().get());

    // end main draw pass
    vulkanCommandBuffer->commandBuffer().endRenderPass();

    // begin debug draw pass
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.setRenderPass(*static_cast<const vk::RenderPass*>(VulkanContext::debugRenderPass->renderPassId()));
    renderPassInfo.setFramebuffer(imageData.debugFramebuffer);
    renderPassInfo.setRenderArea(vk::Rect2D({ 0, 0 }, VulkanContext::swapChainExtent));
    vulkanCommandBuffer->commandBuffer().beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    // draw imgui
    VulkanImGui::draw(commandBuffer());

    // end debug draw pass
    vulkanCommandBuffer->commandBuffer().endRenderPass();
    vulkanCommandBuffer->commandBuffer().end();

    // submit command buffers
    vk::SubmitInfo submitInfo = {};
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(frameData.imageAvailableSemaphore);
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(vulkanCommandBuffer->commandBuffer());
    submitInfo.setSignalSemaphores(frameData.renderFinishedSemaphore);
    VulkanContext::graphicsQueue.submit(submitInfo, frameData.inFlightFence);

    // present swapchain
    uint32_t imageIndex = VulkanContext::currentImage;
    vk::PresentInfoKHR presentInfo = {};
    presentInfo.setWaitSemaphores(frameData.renderFinishedSemaphore);
    presentInfo.setSwapchains(VulkanContext::swapChain);
    presentInfo.setImageIndices(imageIndex);
    vk::Result resultPresent;
    try {
        resultPresent = VulkanContext::presentQueue.presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError&) {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    }

    // check present result
    if (resultPresent == vk::Result::eErrorOutOfDateKHR || resultPresent == vk::Result::eSuboptimalKHR
        || VulkanContext::swapChainInvalidated) {
        VulkanContext::swapChainInvalidated = false;
        VulkanInstance::recreateSwapChain();
    }

    // update current frame
    VulkanContext::currentFrame = (VulkanContext::currentFrame + 1) % VulkanContext::maxFramesInFlight;

    FrameMark;
}

bool VulkanRenderer::debugShowLines() { return VulkanContext::debugShowLines; }

void VulkanRenderer::setDebugShowLines(bool enabled)
{
    // set the debug show lines if needed
    if (VulkanContext::debugShowLines != enabled) {
        VulkanContext::debugShowLines = enabled;
        VulkanContext::dispatcher.trigger<DebugShowLinesEvent>();
    }
}

DescriptorSetLayout VulkanRenderer::descriptorSetLayout()
{
    DescriptorSetLayout descriptorSetLayout = {};
    descriptorSetLayout.setNumber = 0;
    descriptorSetLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 0,
        .descriptorType = DescriptorType::uniformBuffer,
        .descriptorCount = 1,
        .stageFlags = ShaderStage::vertex });
    return descriptorSetLayout;
}

} // namespace chronicle