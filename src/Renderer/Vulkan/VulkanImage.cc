#include "VulkanImage.h"

#include "VulkanBuffer.h"
#include "VulkanRenderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanImage)

VulkanImage::~VulkanImage() { cleanup(); }

void VulkanImage::updateSwapchain(const vk::Image& image, vk::Format format, int width, int height)
{
    CHRZONE_VULKAN

    cleanup();

    _image = image;
    _width = width;
    _height = height;

    createImageView(format, vk::ImageAspectFlagBits::eColor);

    updated();
}

void VulkanImage::updateDepthBuffer(uint32_t width, uint32_t height, vk::Format format)
{
    cleanup();

    createImage(width, height, format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    createImageView(format, vk::ImageAspectFlagBits::eDepth);

    _width = width;
    _height = height;
}

ImageRef VulkanImage::createTexture(const Renderer* renderer, const ImageInfo& imageInfo)
{
    CHRZONE_VULKAN

    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_device = vulkanRenderer->device();
    result->_physicalDevice = vulkanRenderer->physicalDevice();

    int texWidth;
    int texHeight;
    int texChannels;

    // load image
    stbi_uc* pixels = stbi_load(imageInfo.filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(texWidth) * static_cast<vk::DeviceSize>(texHeight) * 4;

    if (!pixels) {
        throw RendererError("Failed to load texture image");
    }

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    VulkanBuffer::create(vulkanRenderer->device(), vulkanRenderer->physicalDevice(), imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    const auto data = vulkanRenderer->device().mapMemory(stagingBufferMemory, 0, imageSize, vk::MemoryMapFlags());
    memcpy(data, pixels, imageSize);
    vulkanRenderer->device().unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    // create vulkan image
    result->createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // copy image buffer
    VulkanBuffer::transitionImageLayout(vulkanRenderer->device(), vulkanRenderer->commandPool(),
        vulkanRenderer->graphicsQueue(), result->_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);
    VulkanBuffer::copyToImage(vulkanRenderer->device(), vulkanRenderer->commandPool(), vulkanRenderer->graphicsQueue(),
        stagingBuffer, result->_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    VulkanBuffer::transitionImageLayout(vulkanRenderer->device(), vulkanRenderer->commandPool(),
        vulkanRenderer->graphicsQueue(), result->_image, vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    vulkanRenderer->device().destroyBuffer(stagingBuffer);
    vulkanRenderer->device().freeMemory(stagingBufferMemory);

    result->_width = texWidth;
    result->_height = texHeight;

    // image view
    result->createImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

    // texture sampler
    result->createTextureSampler();

    return result;
}

ImageRef VulkanImage::createSwapchain(
    const vk::Device& device, const vk::Image& image, vk::Format format, int width, int height)
{
    CHRZONE_VULKAN

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_device = device;
    result->_image = image;
    result->_swapchainImage = true;
    result->_width = width;
    result->_height = height;

    result->createImageView(format, vk::ImageAspectFlagBits::eColor);

    return result;
}

ImageRef VulkanImage::createDepthBuffer(
    const vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, vk::Format format)
{
    CHRZONE_VULKAN

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_device = device;
    result->_physicalDevice = physicalDevice;

    result->createImage(width, height, format, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    result->createImageView(format, vk::ImageAspectFlagBits::eDepth);

    result->_width = width;
    result->_height = height;

    return result;
}

void VulkanImage::cleanup()
{
    CHRZONE_VULKAN

    _device.destroyImageView(_imageView);

    if (!_swapchainImage) {
        _device.destroySampler(_sampler);
        _device.destroyImage(_image);
        _device.freeMemory(_imageMemory);
    }
}

void VulkanImage::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    CHRZONE_VULKAN

    vk::Extent3D imageExtent = {};
    imageExtent.setWidth(width);
    imageExtent.setHeight(height);
    imageExtent.setDepth(1);

    vk::ImageCreateInfo imageCreateInfo {};
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setExtent(imageExtent);
    imageCreateInfo.setMipLevels(1);
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setFormat(format);
    imageCreateInfo.setTiling(tiling);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setUsage(usage);
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

    _image = _device.createImage(imageCreateInfo);

    // allocate memory
    const auto memRequirements = _device.getImageMemoryRequirements(_image);

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(
        VulkanBuffer::findMemoryType(_physicalDevice, memRequirements.memoryTypeBits, properties));

    _imageMemory = _device.allocateMemory(allocInfo, nullptr);

    vkBindImageMemory(_device, _image, _imageMemory, 0);
}

void VulkanImage::createImageView(vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    CHRZONE_VULKAN

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(aspectFlags);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(1);
    subresourceRange.setBaseArrayLayer(0);
    subresourceRange.setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = _image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.setSubresourceRange(subresourceRange);

    _imageView = _device.createImageView(viewInfo);
}

void VulkanImage::createTextureSampler()
{
    CHRZONE_VULKAN

    const auto properties = _physicalDevice.getProperties();

    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.setMagFilter(vk::Filter::eLinear);
    samplerInfo.setMinFilter(vk::Filter::eLinear);
    samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAnisotropyEnable(true);
    samplerInfo.setMaxAnisotropy(properties.limits.maxSamplerAnisotropy);
    samplerInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
    samplerInfo.setUnnormalizedCoordinates(false);
    samplerInfo.setCompareEnable(false);
    samplerInfo.setCompareOp(vk::CompareOp::eAlways);
    samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    samplerInfo.setMipLodBias(0.0f);
    samplerInfo.setMinLod(0.0f);
    samplerInfo.setMaxLod(0.0f);

    _sampler = _device.createSampler(samplerInfo);
}

} // namespace chronicle