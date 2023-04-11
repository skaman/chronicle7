#include "VulkanImage.h"

#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle {

CHR_CONCRETE(VulkanImage)

VulkanImage::~VulkanImage() { cleanup(); }

void VulkanImage::set(void* src, size_t size, uint32_t width, uint32_t height)
{
    CHRZONE_RENDERER

    assert(src != nullptr);
    assert(size > 0);
    assert(width > 0);
    assert(height > 0);

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    VulkanUtils::createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    const auto data = VulkanContext::device.mapMemory(stagingBufferMemory, 0, size, vk::MemoryMapFlags());
    memcpy(data, src, size);
    VulkanContext::device.unmapMemory(stagingBufferMemory);

    // calculate mip levels
    _mipLevels = _generateMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

    // create vulkan image
    auto [imageMemory, image] = VulkanUtils::createImage(width, height, _mipLevels, vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // copy image buffer
    VulkanUtils::transitionImageLayout(
        image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, _mipLevels);
    VulkanUtils::copyBufferToImage(stagingBuffer, image, width, height);

    VulkanContext::device.destroyBuffer(stagingBuffer);
    VulkanContext::device.freeMemory(stagingBufferMemory);

    if (_generateMipmaps) {
        VulkanUtils::generateMipmaps(image, vk::Format::eR8G8B8A8Srgb, width, height, _mipLevels);
    } else {
        VulkanUtils::transitionImageLayout(
            image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, _mipLevels);
    }

    _imageMemory = imageMemory;
    _image = image;
    _imageView
        = VulkanUtils::createImageView(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, _mipLevels);
    _width = width;
    _height = height;
    _sampler = VulkanUtils::createTextureSampler(_mipLevels);
}

void VulkanImage::updateSwapchain(const vk::Image& image, vk::Format format, uint32_t width, uint32_t height)
{
    CHRZONE_RENDERER

    assert(_type == ImageType::Swapchain);
    assert(image);
    assert(width > 0);
    assert(height > 0);

    cleanup();

    _image = image;
    _imageView = VulkanUtils::createImageView(image, format, vk::ImageAspectFlagBits::eColor, 1);
    _width = width;
    _height = height;

    updated();
}

void VulkanImage::updateDepthBuffer(uint32_t width, uint32_t height, vk::Format format)
{
    CHRZONE_RENDERER

    assert(_type == ImageType::Depth);
    assert(width > 0);
    assert(height > 0);
    assert(format != vk::Format::eUndefined);

    cleanup();

    auto [imageMemory, image] = VulkanUtils::createImage(width, height, 1, format, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);

    _imageMemory = imageMemory;
    _image = image;
    _imageView = VulkanUtils::createImageView(image, format, vk::ImageAspectFlagBits::eDepth, 1);
    _width = width;
    _height = height;
}

ImageRef VulkanImage::createTexture(const ImageInfo& imageInfo)
{
    CHRZONE_RENDERER

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_type = ImageType::Texture;
    result->_generateMipmaps = imageInfo.generateMipmaps;
    return result;
}

ImageRef VulkanImage::createSwapchain(const vk::Image& image, vk::Format format, uint32_t width, uint32_t height)
{
    CHRZONE_RENDERER

    assert(image);
    assert(format != vk::Format::eUndefined);
    assert(width > 0);
    assert(height > 0);

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_type = ImageType::Swapchain;
    result->_image = image;
    result->_imageView = VulkanUtils::createImageView(image, format, vk::ImageAspectFlagBits::eColor, 1);
    result->_width = width;
    result->_height = height;
    return result;
}

ImageRef VulkanImage::createDepthBuffer(uint32_t width, uint32_t height, vk::Format format)
{
    CHRZONE_RENDERER

    assert(width > 0);
    assert(height > 0);
    assert(format != vk::Format::eUndefined);

    auto [imageMemory, image] = VulkanUtils::createImage(width, height, 1, format, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_type = ImageType::Depth;
    result->_imageMemory = imageMemory;
    result->_image = image;
    result->_imageView = VulkanUtils::createImageView(image, format, vk::ImageAspectFlagBits::eDepth, 1);
    result->_width = width;
    result->_height = height;
    return result;
}

void VulkanImage::cleanup() const
{
    CHRZONE_RENDERER

    VulkanContext::device.destroyImageView(_imageView);

    if (_type != ImageType::Swapchain) {
        VulkanContext::device.destroySampler(_sampler);
        VulkanContext::device.destroyImage(_image);
        VulkanContext::device.freeMemory(_imageMemory);
    }
}

} // namespace chronicle