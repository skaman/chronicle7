#include "VulkanRenderPass.h"

#include "VulkanImage.h"
#include "VulkanRenderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanRenderPass)

VulkanRenderPass::VulkanRenderPass(const vk::Device& device, const RenderPassInfo& renderPassInfo)
    : _device(device)
    , _depthImage(renderPassInfo.depthImage)
{
    CHRZONE_VULKAN

    // color attachment
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

    // depth attachment
    vk::AttachmentDescription depthAttachment = {};
    depthAttachment.setFormat(formatToVulkan(renderPassInfo.depthAttachmentFormat));
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.setAttachment(1);
    depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    // subpass
    vk::SubpassDescription subpass = {};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachments(colorAttachmentRef);
    subpass.setPDepthStencilAttachment(&depthAttachmentRef);

    // dependency
    vk::SubpassDependency dependency = {};
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDstSubpass(0);
    dependency.setSrcStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    dependency.setDstStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    dependency.setDstAccessMask(
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    // render pass
    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    vk::RenderPassCreateInfo createRenderPassInfo = {};
    createRenderPassInfo.setAttachments(attachments);
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

        const auto vulkanImage = static_cast<VulkanImage*>(image.get());

        vulkanImage->updated.connect<&VulkanRenderPass::imageUpdatedEvent>(&_updateData[i]);

        _images.push_back(image);
        _framebuffers.push_back(createFrameBuffer(image, renderPassInfo.depthImage));
    }
}

VulkanRenderPass::~VulkanRenderPass()
{
    CHRZONE_VULKAN

    for (const auto& image : _images) {
        const auto vulkanImage = static_cast<VulkanImage*>(image.get());

        vulkanImage->updated.reset();
    }

    _images.clear();

    for (const auto& framebuffer : _framebuffers)
        _device.destroyFramebuffer(framebuffer);

    _device.destroyRenderPass(_renderPass);
}

RenderPassRef VulkanRenderPass::create(const Renderer* renderer, const RenderPassInfo& renderPassInfo)
{
    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);

    return std::make_shared<ConcreteVulkanRenderPass>(vulkanRenderer->device(), renderPassInfo);
}

vk::Framebuffer VulkanRenderPass::createFrameBuffer(const ImageRef& image, const ImageRef& depthImage) const
{
    CHRZONE_VULKAN

    const auto vulkanImage = static_cast<const VulkanImage*>(image.get());
    const auto vulkanDepthImage = static_cast<const VulkanImage*>(depthImage.get());

    std::array<vk::ImageView, 2> attachments = { vulkanImage->imageView(), vulkanDepthImage->imageView() };

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.setRenderPass(_renderPass);
    framebufferInfo.setAttachments(attachments);
    framebufferInfo.setWidth(vulkanImage->width());
    framebufferInfo.setHeight(vulkanImage->height());
    framebufferInfo.setLayers(1);

    return _device.createFramebuffer(framebufferInfo);
}

void VulkanRenderPass::recreateFrameBuffer(uint32_t imageIndex)
{
    CHRZONE_VULKAN

    _device.destroyFramebuffer(_framebuffers[imageIndex]);

    _framebuffers[imageIndex] = createFrameBuffer(_images[imageIndex], _depthImage);
}

void VulkanRenderPass::imageUpdatedEvent(ImageUpdateData* data) { data->renderPass->recreateFrameBuffer(data->index); }

} // namespace chronicle