// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTexture.h"

#include "VulkanEnums.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle::internal::vulkan {

CHR_CONCRETE(VulkanTexture);

VulkanTexture::VulkanTexture(const SampledTextureInfo& textureInfo, const std::string& name)
    : _name(name)
    , _format(Format::R8G8B8A8Unorm)
    , _generateMipmaps(textureInfo.generateMipmaps)
    , _width(textureInfo.width)
    , _height(textureInfo.height)
{
    CHRZONE_RENDERER;

    assert(_width > 0);
    assert(_height > 0);

    if (!textureInfo.data.empty()) {
        // create a buffer visible to the host
        auto [stagingBufferMemory, stagingBuffer]
            = VulkanUtils::createBuffer(textureInfo.data.size(), vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        // copy image data into the buffer
        const auto data
            = VulkanContext::device.mapMemory(stagingBufferMemory, 0, textureInfo.data.size(), vk::MemoryMapFlags());
        std::memcpy(data, textureInfo.data.data(), textureInfo.data.size());
        VulkanContext::device.unmapMemory(stagingBufferMemory);

        // calculate mip levels
        _mipLevels = _generateMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height)))) + 1 : 1;

        // create vulkan image
        auto [imageMemory, image] = VulkanUtils::createImage(_width, _height, _mipLevels, vk::SampleCountFlagBits::e1,
            vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
                | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

        // copy local buffer to the image buffer
        VulkanUtils::transitionImageLayout(
            image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, _mipLevels);
        VulkanUtils::copyBufferToImage(stagingBuffer, image, _width, _height);

        // destroy and free memory of local visible buffer
        VulkanContext::device.destroyBuffer(stagingBuffer);
        VulkanContext::device.freeMemory(stagingBufferMemory);

        // generate mipmaps if required, or just trasition the image layout
        if (_generateMipmaps) {
            VulkanUtils::generateMipmaps(image, vk::Format::eR8G8B8A8Unorm, _width, _height, _mipLevels);
        } else {
            VulkanUtils::transitionImageLayout(
                image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, _mipLevels);
        }

        // stora image data
        _imageMemory = imageMemory;
        _image = image;

        // create image view
        _imageView = VulkanUtils::createImageView(
            image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, _mipLevels);

        // create sampler
        _sampler = VulkanUtils::createTextureSampler(_mipLevels);
    }
}

VulkanTexture::VulkanTexture(const ColorTextureInfo& textureInfo, const std::string& name)
    : _name(name)
    , _type(TextureType::color)
    , _format(textureInfo.format)
    , _generateMipmaps(textureInfo.generateMipmaps)
    , _width(textureInfo.width)
    , _height(textureInfo.height)
{
    CHRZONE_RENDERER;

    assert(_width > 0);
    assert(_height > 0);
    assert(_format != Format::undefined);

    auto format = VulkanEnums::formatToVulkan(_format);

    vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
    if (textureInfo.isInputAttachment) {
        usageFlags |= vk::ImageUsageFlagBits::eInputAttachment;
    }

    // calculate mip levels
    _mipLevels = _generateMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height)))) + 1 : 1;

    auto [imageMemory, image]
        = VulkanUtils::createImage(_width, _height, _mipLevels, VulkanEnums::msaaToVulkan(textureInfo.msaa), format,
            vk::ImageTiling::eOptimal, usageFlags, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _imageMemory = imageMemory;
    _image = image;
    _imageView = VulkanUtils::createImageView(image, format, vk::ImageAspectFlagBits::eColor, _mipLevels);

    // create sampler
    _sampler = VulkanUtils::createTextureSampler(_mipLevels);
}

VulkanTexture::VulkanTexture(const DepthTextureInfo& textureInfo, const std::string& name)
    : _name(name)
    , _type(TextureType::depth)
    , _format(textureInfo.format)
    , _width(textureInfo.width)
    , _height(textureInfo.height)
{
    CHRZONE_RENDERER;

    assert(_width > 0);
    assert(_height > 0);
    assert(_format != Format::undefined);

    auto format = VulkanEnums::formatToVulkan(_format);

    auto [imageMemory, image] = VulkanUtils::createImage(_width, _height, 1,
        VulkanEnums::msaaToVulkan(textureInfo.msaa), format, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    _imageMemory = imageMemory;
    _image = image;
    _imageView = VulkanUtils::createImageView(image, format, vk::ImageAspectFlagBits::eDepth, 1);
}

VulkanTexture::VulkanTexture(
    const vk::Image& image, vk::Format format, uint32_t width, uint32_t height, const std::string& name)
    : _name(name)
    , _image(image)
    , _type(TextureType::swapchain)
    , _width(width)
    , _height(height)
{
    CHRZONE_RENDERER;

    assert(format != vk::Format::eUndefined);
    assert(_image);
    assert(_width > 0);
    assert(_height > 0);

    _imageView = VulkanUtils::createImageView(_image, format, vk::ImageAspectFlagBits::eColor, 1);
}

VulkanTexture::~VulkanTexture()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Destroy texture");

    VulkanContext::device.destroyImageView(_imageView);

    if (_sampler) {
        VulkanContext::device.destroySampler(_sampler);
    }

    if (_type != TextureType::swapchain) {
        VulkanContext::device.destroyImage(_image);
        VulkanContext::device.freeMemory(_imageMemory);
    }
}

TextureRef VulkanTexture::createSampled(const SampledTextureInfo& textureInfo, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanTexture>(textureInfo, name);
}

TextureRef VulkanTexture::createColor(const ColorTextureInfo& textureInfo, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanTexture>(textureInfo, name);
}

TextureRef VulkanTexture::createDepth(const DepthTextureInfo& textureInfo, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanTexture>(textureInfo, name);
}

TextureRef VulkanTexture::createSwapchain(
    const vk::Image& image, vk::Format format, uint32_t width, uint32_t height, const std::string& name)
{
    CHRZONE_RENDERER;

    // create an instance of the class
    return std::make_shared<ConcreteVulkanTexture>(image, format, width, height, name);
}

} // namespace chronicle