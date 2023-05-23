// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanRenderer.h"

#include "VulkanCommandBuffer.h"
#include "VulkanEvents.h"
#include "VulkanFrameBuffer.h"
#include "VulkanGC.h"
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

    CHRLOG_TRACE("Begin frame");

    // get the current frame data
    const VulkanFrameData& frameData = VulkanContext::framesData[VulkanContext::currentFrame];

    // wait for fence (image GPU processing completed)
    (void)VulkanContext::device.waitForFences(frameData.inFlightFence, true, std::numeric_limits<uint64_t>::max());

    // reset the fence
    VulkanContext::device.resetFences(frameData.inFlightFence);

    // clean the frame garbage collector
    VulkanGC::cleanupCurrentQueue();

    // acquire the image
    try {
        auto result = VulkanContext::device.acquireNextImageKHR(
            VulkanContext::swapChain, std::numeric_limits<uint64_t>::max(), frameData.imageAvailableSemaphore, nullptr);
        VulkanContext::currentImage = result.value;
    } catch (const vk::OutOfDateKHRError&) {
        VulkanInstance::recreateSwapChain();
        return false;
    }

    // new imgui frame
    VulkanImGui::newFrame();

    CHRLOG_TRACE(
        "New frame: area extent={}x{}", VulkanContext::swapChainExtent.width, VulkanContext::swapChainExtent.height);

    // begin main command buffer
    commandBuffer()->begin();

    return true;
}

void VulkanRenderer::endFrame()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("End frame");

    auto vulkanCommandBuffer = commandBuffer()->commandBufferId();

    // get the current frame data
    const VulkanFrameData& frameData = VulkanContext::framesData[VulkanContext::currentFrame];

    commandBuffer()->end();

    // submit command buffers
    vk::SubmitInfo submitInfo = {};
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(frameData.imageAvailableSemaphore);
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.setCommandBuffers(vulkanCommandBuffer);
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

void VulkanRenderer::beginRenderPass()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Begin render pass");

    // get the current image data
    const VulkanImageData& imageData = VulkanContext::imagesData[VulkanContext::currentImage];

    // begin render pass
    commandBuffer()->beginRenderPass({ .renderPassId = VulkanContext::renderPass->renderPassId(),
        .frameBufferId = imageData.framebuffer->frameBufferId(),
        .renderAreaOffset = { 0, 0 },
        .renderAreaExtent = { VulkanContext::swapChainExtent.width, VulkanContext::swapChainExtent.height } });

    // set viewport
    commandBuffer()->setViewport({ .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(VulkanContext::swapChainExtent.width),
        .height = static_cast<float>(VulkanContext::swapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f });
}

void VulkanRenderer::endRenderPass()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("End render pass");

    // draw imgui
    VulkanImGui::draw(commandBuffer()->commandBufferId());

    // end debug draw pass
    commandBuffer()->endRenderPass();
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