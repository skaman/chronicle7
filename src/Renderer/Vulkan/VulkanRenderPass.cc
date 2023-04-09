#include "VulkanRenderPass.h"

#include "Renderer/Renderer.h"

#include "Renderer/Image.h"
#include "Renderer/RenderPassInfo.h"
#include "VulkanCommon.h"
#include "VulkanRenderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanRenderPass)

VulkanRenderPass::VulkanRenderPass(const vk::Device& device, const RenderPassInfo& renderPassInfo)
    : _device(device)
{
    CHRZONE_VULKAN

    // render pass
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.setFormat(formatToVulkan(renderPassInfo.colorAttachmentFormat));
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.setAttachment(0);
    colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachments(colorAttachmentRef);

    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDstSubpass(0);
    dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo createRenderPassInfo = {};
    createRenderPassInfo.setAttachments(colorAttachment);
    createRenderPassInfo.setSubpasses(subpass);
    createRenderPassInfo.setDependencies(dependency);

    _renderPass = _device.createRenderPass(createRenderPassInfo);

    // framebuffers
    _framebuffers.reserve(renderPassInfo.images.size());
    _updateData.resize(renderPassInfo.images.size());

    for (uint32_t i = 0; i < renderPassInfo.images.size(); i++) {
        const auto& image = renderPassInfo.images[i];
        _updateData[i].index = i;
        _updateData[i].renderPass = this;
        image->native().updated.connect<&VulkanRenderPass::imageUpdatedEvent>(&_updateData[i]);

        _images.push_back(image);
        _framebuffers.push_back(createFrameBuffer(image));
    }
}

VulkanRenderPass::~VulkanRenderPass()
{
    CHRZONE_VULKAN

    for (const auto& image : _images)
        image->native().updated.reset();
    _images.clear();

    for (const auto& framebuffer : _framebuffers)
        _device.destroyFramebuffer(framebuffer);

    _device.destroyRenderPass(_renderPass);
}

RenderPassRef VulkanRenderPass::create(const Renderer* renderer, const RenderPassInfo& renderPassInfo)
{
    return std::make_shared<ConcreteVulkanRenderPass>(renderer->native().device(), renderPassInfo);
}

vk::Framebuffer VulkanRenderPass::createFrameBuffer(const std::shared_ptr<Image>& image) const
{
    CHRZONE_VULKAN

    const auto& vulkanImage = image->native();

    std::array<vk::ImageView, 1> attachments = { vulkanImage.imageView() };

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(_renderPass);
    framebufferInfo.setAttachments(attachments);
    framebufferInfo.setWidth(vulkanImage.width());
    framebufferInfo.setHeight(vulkanImage.height());
    framebufferInfo.setLayers(1);

    return _device.createFramebuffer(framebufferInfo);
}

void VulkanRenderPass::recreateFrameBuffer(uint32_t imageIndex)
{
    CHRZONE_VULKAN

    _device.destroyFramebuffer(_framebuffers[imageIndex]);

    _framebuffers[imageIndex] = createFrameBuffer(_images[imageIndex]);
}

void VulkanRenderPass::imageUpdatedEvent(ImageUpdateData* data) { data->renderPass->recreateFrameBuffer(data->index); }

} // namespace chronicle