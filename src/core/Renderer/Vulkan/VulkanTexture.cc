// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTexture.h"

#include "VulkanInstance.h"
#include "VulkanUtils.h"

namespace chronicle {

CHR_CONCRETE(VulkanTexture);

VulkanTexture::~VulkanTexture()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy texture");

    cleanup();
}

void VulkanTexture::set(void* src, size_t size, uint32_t width, uint32_t height)
{
    CHRZONE_RENDERER;

    assert(src != nullptr);
    assert(size > 0);
    assert(width > 0);
    assert(height > 0);

    CHRLOG_DEBUG("Set texture data: size={}, width={}, height={}", size, width, height);

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
    auto [imageMemory, image] = VulkanUtils::createImage(width, height, _mipLevels, vk::Format::eR8G8B8A8Unorm,
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
        VulkanUtils::generateMipmaps(image, vk::Format::eR8G8B8A8Unorm, width, height, _mipLevels);
    } else {
        VulkanUtils::transitionImageLayout(
            image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, _mipLevels);
    }

    _imageMemory = imageMemory;
    _image = image;
    _imageView
        = VulkanUtils::createImageView(image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, _mipLevels);
    _width = width;
    _height = height;
    _sampler = VulkanUtils::createTextureSampler(_mipLevels);
}

TextureRef VulkanTexture::create(const TextureInfo& textureInfo)
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create texture: generate mipmaps={}", textureInfo.generateMipmaps);

    auto result = std::make_shared<ConcreteVulkanTexture>();
    result->_generateMipmaps = textureInfo.generateMipmaps;
    return result;
}

void VulkanTexture::cleanup() const
{
    CHRZONE_RENDERER;

    VulkanContext::device.destroyImageView(_imageView);
    VulkanContext::device.destroySampler(_sampler);
    VulkanContext::device.destroyImage(_image);
    VulkanContext::device.freeMemory(_imageMemory);
}

} // namespace chronicle