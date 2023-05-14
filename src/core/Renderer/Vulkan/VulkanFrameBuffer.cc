// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanFrameBuffer.h"

#include "VulkanCommon.h"

namespace chronicle {

CHR_CONCRETE(VulkanFrameBuffer);

VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferInfo& frameBufferInfo)
{
    CHRZONE_RENDERER;

    assert(!frameBufferInfo.attachments.empty());
    assert(frameBufferInfo.renderPass);
    assert(frameBufferInfo.width > 0);
    assert(frameBufferInfo.height > 0);

    std::vector<vk::ImageView> attachments(frameBufferInfo.attachments.size());
    for (auto i = 0; i < frameBufferInfo.attachments.size(); i++) {
        attachments[i] = *static_cast<const vk::ImageView*>(frameBufferInfo.attachments[i]);
    }

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(*static_cast<const vk::RenderPass*>(frameBufferInfo.renderPass));
    framebufferInfo.setAttachments(attachments);
    framebufferInfo.setWidth(frameBufferInfo.width);
    framebufferInfo.setHeight(frameBufferInfo.height);
    framebufferInfo.setLayers(1);
    _framebuffer = VulkanContext::device.createFramebuffer(framebufferInfo);
}

VulkanFrameBuffer::~VulkanFrameBuffer()
{
    CHRZONE_RENDERER;

    VulkanContext::device.destroyFramebuffer(_framebuffer);
}

FrameBufferRef VulkanFrameBuffer::create(const FrameBufferInfo& frameBufferInfo)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanFrameBuffer>(frameBufferInfo);
}

} // namespace chronicle