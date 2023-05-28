// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanRenderPass.h"

#include "VulkanCommon.h"
#include "VulkanEnums.h"
#include "VulkanUtils.h"

namespace chronicle::internal::vulkan {

CHR_CONCRETE(VulkanRenderPass);

VulkanRenderPass::VulkanRenderPass(const RenderPassInfo& renderPassInfo, const std::string& name)
    : _name(name)
    , _format(renderPassInfo.colorAttachment.format)
    , _msaa(renderPassInfo.colorAttachment.msaa)
{
    CHRZONE_RENDERER;

    _hash = std::hash<RenderPassInfo>()(renderPassInfo);

    std::vector<vk::AttachmentDescription> attachments = {};

    // subpass
    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    // color attachment
    const auto& colorAttachment = renderPassInfo.colorAttachment;

    auto colorAttachmentRef = createAttachmentReference(static_cast<uint32_t>(attachments.size()), false);
    subpass.setColorAttachments(colorAttachmentRef);

    attachments.push_back(createAttachmentDescription(colorAttachment));

    // depth attachment
    vk::AttachmentReference depthStencilAttachmentRef;
    if (renderPassInfo.depthStencilAttachment) {
        const auto& attachment = renderPassInfo.depthStencilAttachment.value();

        depthStencilAttachmentRef = createAttachmentReference(static_cast<uint32_t>(attachments.size()), true);
        subpass.setPDepthStencilAttachment(&depthStencilAttachmentRef);

        attachments.push_back(createAttachmentDescription(attachment));
    }

    // resolve attachment
    vk::AttachmentReference resolveAttachmentRef;
    if (renderPassInfo.resolveAttachment) {
        const auto& attachment = renderPassInfo.resolveAttachment.value();

        resolveAttachmentRef = createAttachmentReference(static_cast<uint32_t>(attachments.size()), false);
        subpass.setPResolveAttachments(&resolveAttachmentRef);

        attachments.push_back(createAttachmentDescription(attachment));
    }

    // dependency
    vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::AccessFlags dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    if (renderPassInfo.depthStencilAttachment) {
        srcStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dstAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    }

    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDstSubpass(0);
    dependency.setSrcStageMask(srcStageMask);
    dependency.setDstStageMask(dstStageMask);
    dependency.setDstAccessMask(dstAccessMask);

    // create the renderpass
    vk::RenderPassCreateInfo createRenderPassInfo = {};
    createRenderPassInfo.setAttachments(attachments);
    createRenderPassInfo.setSubpasses(subpass);
    createRenderPassInfo.setDependencies(dependency);

    _renderPass = VulkanContext::device.createRenderPass(createRenderPassInfo);

    assert(_renderPass);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::setDebugObjectName(_renderPass, _name);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

VulkanRenderPass::~VulkanRenderPass()
{
    CHRZONE_RENDERER;

    VulkanContext::device.destroyRenderPass(_renderPass);
}

RenderPassRef VulkanRenderPass::create(const RenderPassInfo& renderPassInfo, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanRenderPass>(renderPassInfo, name);
}

vk::AttachmentDescription VulkanRenderPass::createAttachmentDescription(const RenderPassAttachment& attachment) const
{
    vk::AttachmentDescription attachmentDescription = {};
    attachmentDescription.setFormat(VulkanEnums::formatToVulkan(attachment.format));
    attachmentDescription.setSamples(VulkanEnums::msaaToVulkan(attachment.msaa));
    attachmentDescription.setLoadOp(VulkanEnums::attachmentLoadOpToVulkan(attachment.loadOp));
    attachmentDescription.setStoreOp(VulkanEnums::attachmentStoreOpToVulkan(attachment.storeOp));
    attachmentDescription.setStencilLoadOp(VulkanEnums::attachmentLoadOpToVulkan(attachment.stencilLoadOp));
    attachmentDescription.setStencilStoreOp(VulkanEnums::attachmentStoreOpToVulkan(attachment.stencilStoreOp));
    attachmentDescription.setInitialLayout(VulkanEnums::imageLayoutToVulkan(attachment.initialLayout));
    attachmentDescription.setFinalLayout(VulkanEnums::imageLayoutToVulkan(attachment.finalLayout));
    return attachmentDescription;
}

vk::AttachmentReference VulkanRenderPass::createAttachmentReference(uint32_t index, bool isDepthAttachment) const
{
    // we use the final layout for the reference layout, with the exception of the swapchain, in that case we use
    // the color attachment layout
    auto attachmentReferenceLayout = isDepthAttachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal
                                                       : vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference attachmentReference = {};
    attachmentReference.setAttachment(index);
    attachmentReference.setLayout(attachmentReferenceLayout);
    return attachmentReference;
}

} // namespace chronicle