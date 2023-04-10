#include "VulkanImage.h"

#include "VulkanBuffer.h"
#include "VulkanImageUtils.h"
#include "VulkanRenderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanImage)

VulkanImage::~VulkanImage() { cleanup(); }

void VulkanImage::set(void* src, size_t size, uint32_t width, uint32_t height)
{
    CHRZONE_VULKAN

    assert(src != nullptr);
    assert(size > 0);
    assert(width > 0);
    assert(height > 0);

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    VulkanBuffer::create(_device, _physicalDevice, size, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    const auto data = _device.mapMemory(stagingBufferMemory, 0, size, vk::MemoryMapFlags());
    memcpy(data, src, size);
    _device.unmapMemory(stagingBufferMemory);

    // calculate mip levels
    _mipLevels = _generateMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

    // create vulkan image
    auto [imageMemory, image] = VulkanImageUtils::createImage(_device, _physicalDevice, width, height, _mipLevels,
        vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // copy image buffer
    VulkanBuffer::transitionImageLayout(_device, _commandPool, _queue, image, vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
    VulkanBuffer::copyToImage(_device, _commandPool, _queue, stagingBuffer, image, width, height);
    VulkanBuffer::transitionImageLayout(_device, _commandPool, _queue, image, vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, _mipLevels);

    _device.destroyBuffer(stagingBuffer);
    _device.freeMemory(stagingBufferMemory);

    _width = width;
    _height = height;
    _imageMemory = imageMemory;
    _image = image;

    // image view
    _imageView
        = VulkanImageUtils::createImageView(_device, image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

    // texture sampler
    _sampler = VulkanImageUtils::createTextureSampler(_device, _physicalDevice);
}

void VulkanImage::updateSwapchain(const vk::Image& image, vk::Format format, uint32_t width, uint32_t height)
{
    CHRZONE_VULKAN

    assert(_type == ImageType::Swapchain);
    assert(image);
    assert(width > 0);
    assert(height > 0);

    cleanup();

    _image = image;
    _width = width;
    _height = height;

    _imageView = VulkanImageUtils::createImageView(_device, image, format, vk::ImageAspectFlagBits::eColor);

    updated();
}

void VulkanImage::updateDepthBuffer(uint32_t width, uint32_t height, vk::Format format)
{
    CHRZONE_VULKAN

    assert(_type == ImageType::Depth);
    assert(width > 0);
    assert(height > 0);
    assert(format != vk::Format::eUndefined);

    cleanup();

    auto [imageMemory, image]
        = VulkanImageUtils::createImage(_device, _physicalDevice, width, height, 1, format, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _imageMemory = imageMemory;
    _image = image;

    _imageView = VulkanImageUtils::createImageView(_device, image, format, vk::ImageAspectFlagBits::eDepth);

    _width = width;
    _height = height;
}

ImageRef VulkanImage::createTexture(const Renderer* renderer, const ImageInfo& imageInfo)
{
    CHRZONE_VULKAN

    assert(renderer != nullptr);

    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_type = ImageType::Texture;
    result->_device = vulkanRenderer->device();
    result->_physicalDevice = vulkanRenderer->physicalDevice();
    result->_commandPool = vulkanRenderer->commandPool();
    result->_queue = vulkanRenderer->graphicsQueue();
    result->_generateMipmaps = imageInfo.generateMipmaps;
    return result;
}

ImageRef VulkanImage::createSwapchain(
    const vk::Device& device, const vk::Image& image, vk::Format format, uint32_t width, uint32_t height)
{
    CHRZONE_VULKAN

    assert(device);
    assert(image);
    assert(format != vk::Format::eUndefined);
    assert(width > 0);
    assert(height > 0);

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_type = ImageType::Swapchain;
    result->_device = device;
    result->_image = image;
    result->_width = width;
    result->_height = height;

    result->_imageView = VulkanImageUtils::createImageView(device, image, format, vk::ImageAspectFlagBits::eColor);

    return result;
}

ImageRef VulkanImage::createDepthBuffer(
    const vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, vk::Format format)
{
    CHRZONE_VULKAN

    assert(device);
    assert(physicalDevice);
    assert(width > 0);
    assert(height > 0);
    assert(format != vk::Format::eUndefined);

    auto result = std::make_shared<ConcreteVulkanImage>();
    result->_type = ImageType::Depth;
    result->_device = device;
    result->_physicalDevice = physicalDevice;

    auto [imageMemory, image]
        = VulkanImageUtils::createImage(device, physicalDevice, width, height, 1, format, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    result->_imageMemory = imageMemory;
    result->_image = image;
    result->_imageView = VulkanImageUtils::createImageView(device, image, format, vk::ImageAspectFlagBits::eDepth);

    result->_width = width;
    result->_height = height;

    return result;
}

void VulkanImage::cleanup() const
{
    CHRZONE_VULKAN

    _device.destroyImageView(_imageView);

    if (_type != ImageType::Swapchain) {
        _device.destroySampler(_sampler);
        _device.destroyImage(_image);
        _device.freeMemory(_imageMemory);
    }
}

} // namespace chronicle