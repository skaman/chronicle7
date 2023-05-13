// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanRenderPass.h"

#include "VulkanCommon.h"
#include "VulkanEnums.h"

namespace chronicle {

CHR_CONCRETE(VulkanRenderPass);

VulkanRenderPass::VulkanRenderPass(const RenderPassInfo& renderPassInfo)
{
    CHRZONE_RENDERER;

    assert(renderPassInfo.colorAttachment);

    std::vector<vk::AttachmentDescription> attachments = {};

    // subpass
    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

    // color attachment
    if (renderPassInfo.colorAttachment) {
        const auto& attachment = renderPassInfo.colorAttachment.value();

        auto attachmentRef = createAttachmentReference(attachment, static_cast<uint32_t>(attachments.size()));
        subpass.setColorAttachments(attachmentRef);

        attachments.push_back(createAttachmentDescription(attachment));
    }

    // depth attachment
    if (renderPassInfo.depthStencilAttachment) {
        const auto& attachment = renderPassInfo.depthStencilAttachment.value();

        auto attachmentRef = createAttachmentReference(attachment, static_cast<uint32_t>(attachments.size()));
        subpass.setPDepthStencilAttachment(&attachmentRef);

        attachments.push_back(createAttachmentDescription(attachment));
    }

    // resolve attachment
    if (renderPassInfo.resolveAttachment) {
        const auto& attachment = renderPassInfo.resolveAttachment.value();

        auto attachmentRef = createAttachmentReference(attachment, static_cast<uint32_t>(attachments.size()));
        subpass.setPResolveAttachments(&attachmentRef);

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
}

VulkanRenderPass::~VulkanRenderPass()
{
    CHRZONE_RENDERER;

    VulkanContext::device.destroyRenderPass(_renderPass);
}

RenderPassRef VulkanRenderPass::create(const RenderPassInfo& renderPassInfo)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanRenderPass>(renderPassInfo);
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

vk::AttachmentReference VulkanRenderPass::createAttachmentReference(
    const RenderPassAttachment& attachment, uint32_t index) const
{
    // we use the final layout for the reference layout, with the exception of the swapchain, in that case we use
    // the color attachment layout
    auto attachmentReferenceLayout = attachment.finalLayout == ImageLayout::presentSrc
        ? vk::ImageLayout::eColorAttachmentOptimal
        : VulkanEnums::imageLayoutToVulkan(attachment.finalLayout);

    vk::AttachmentReference attachmentReference = {};
    attachmentReference.setAttachment(index);
    attachmentReference.setLayout(attachmentReferenceLayout);
    return attachmentReference;
}

} // namespace chronicle