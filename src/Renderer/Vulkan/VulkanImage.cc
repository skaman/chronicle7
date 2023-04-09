#include "VulkanImage.h"

#include "VulkanBuffer.h"
#include "VulkanRenderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanImage)

VulkanImage::VulkanImage(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
    const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue, const ImageInfo& imageInfo)
    : _device(device)
    , _physicalDevice(physicalDevice)
{
    CHRZONE_VULKAN

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

    VulkanBuffer::create(_device, physicalDevice, imageSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    const auto data = device.mapMemory(stagingBufferMemory, 0, imageSize, vk::MemoryMapFlags());
    memcpy(data, pixels, imageSize);
    device.unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    // create vulkan image
    createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // copy image buffer
    VulkanBuffer::transitionImageLayout(_device, commandPool, graphicsQueue, _image, vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    VulkanBuffer::copyToImage(_device, commandPool, graphicsQueue, stagingBuffer, _image,
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    VulkanBuffer::transitionImageLayout(_device, commandPool, graphicsQueue, _image, vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    device.destroyBuffer(stagingBuffer);
    device.freeMemory(stagingBufferMemory);

    _width = texWidth;
    _height = texHeight;

    // image view
    createImageView(vk::Format::eR8G8B8A8Srgb);

    // texture sampler
    createTextureSampler();
}

VulkanImage::VulkanImage(const vk::Device& device, const vk::Image& image, vk::Format format, int width, int height)
    : _device(device)
    , _image(image)
    , _swapchainImage(true)
    , _width(width)
    , _height(height)
{
    CHRZONE_VULKAN

    createImageView(format);
}

VulkanImage::~VulkanImage()
{
    CHRZONE_VULKAN

    _device.destroyImageView(_imageView);

    if (!_swapchainImage) {
        _device.destroySampler(_sampler);
        _device.destroyImage(_image);
        _device.freeMemory(_imageMemory);
    }
}

void VulkanImage::updateImage(const vk::Image& image, vk::Format format, int width, int height)
{
    CHRZONE_VULKAN

    _device.destroyImageView(_imageView);

    _image = image;
    _width = width;
    _height = height;

    createImageView(format);

    updated();
}

ImageRef VulkanImage::create(const Renderer* renderer, const ImageInfo& imageInfo)
{
    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);
    return std::make_shared<ConcreteVulkanImage>(vulkanRenderer->device(), vulkanRenderer->physicalDevice(),
        vulkanRenderer->commandPool(), vulkanRenderer->graphicsQueue(), imageInfo);
}

ImageRef VulkanImage::create(const vk::Device& device, const vk::Image& image, vk::Format format, int width, int height)
{
    return std::make_shared<ConcreteVulkanImage>(device, image, format, width, height);
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

void VulkanImage::createImageView(vk::Format format)
{
    CHRZONE_VULKAN

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
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