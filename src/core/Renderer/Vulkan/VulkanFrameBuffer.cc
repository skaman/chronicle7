// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanFrameBuffer.h"

#include "VulkanCommon.h"
#include "VulkanUtils.h"

namespace chronicle {

CHR_CONCRETE(VulkanFrameBuffer);

VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferInfo& frameBufferInfo, const std::string& name)
    : _name(name)
{
    CHRZONE_RENDERER;

    assert(!frameBufferInfo.attachments.empty());
    assert(frameBufferInfo.renderPass);
    assert(frameBufferInfo.width > 0);
    assert(frameBufferInfo.height > 0);

    std::vector<vk::ImageView> attachments(frameBufferInfo.attachments.size());
    for (auto i = 0; i < frameBufferInfo.attachments.size(); i++) {
        attachments[i] = frameBufferInfo.attachments[i];
    }

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(frameBufferInfo.renderPass);
    framebufferInfo.setAttachments(attachments);
    framebufferInfo.setWidth(frameBufferInfo.width);
    framebufferInfo.setHeight(frameBufferInfo.height);
    framebufferInfo.setLayers(1);
    _framebuffer = VulkanContext::device.createFramebuffer(framebufferInfo);

    assert(_framebuffer);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::setDebugObjectName(_framebuffer, name);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

VulkanFrameBuffer::~VulkanFrameBuffer()
{
    CHRZONE_RENDERER;

    VulkanContext::device.destroyFramebuffer(_framebuffer);
}

FrameBufferRef VulkanFrameBuffer::create(const FrameBufferInfo& frameBufferInfo, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanFrameBuffer>(frameBufferInfo, name);
}

} // namespace chronicle