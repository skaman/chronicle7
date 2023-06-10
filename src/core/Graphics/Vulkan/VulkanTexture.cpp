// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTexture.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanTextureView.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

VulkanTexture::VulkanTexture(std::shared_ptr<VulkanDevice> device, const TextureDescriptor &textureDescriptor)
    : Texture(textureDescriptor), _device(device)
{
    // Create the texture.
    try
    {
        vk::Extent3D imageExtent(descriptor().width, descriptor().height, descriptor().depth);

        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.setImageType(convertImageType(descriptor().dimension));
        imageCreateInfo.setExtent(imageExtent);
        imageCreateInfo.setMipLevels(descriptor().mipLevelCount);
        imageCreateInfo.setArrayLayers(descriptor().arrayLayers);
        imageCreateInfo.setFormat(convertFormat(descriptor().format));
        imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
        imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
        imageCreateInfo.setUsage(convertUsage(descriptor().usage));
        imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
        imageCreateInfo.setSamples(convertSamples(descriptor().sampleCount));

        _vulkanImage = _device->vulkanLogicalDevice().createImage(imageCreateInfo);
    }
    catch (const vk::Error &error)
    {
        throw TextureError(fmt::format("Can't create the texture: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eImage, (uint64_t)(VkImage)_vulkanImage, descriptor().name);
#endif

    /// Allocate memory.
    try
    {
        const auto memRequirements = _device->vulkanLogicalDevice().getImageMemoryRequirements(_vulkanImage);

        vk::MemoryAllocateInfo allocInfo(
            memRequirements.size,
            _device->findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
        _vulkanMemory = _device->vulkanLogicalDevice().allocateMemory(allocInfo);
    }
    catch (const vk::Error &error)
    {
        _device->vulkanLogicalDevice().destroyImage(_vulkanImage);
        throw TextureError(fmt::format("Can't allocate memory for the buffer: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eDeviceMemory, (uint64_t)(VkDeviceMemory)_vulkanMemory,
                               descriptor().name);
#endif

    /// Bind memory and texture together.
    try
    {
        _device->vulkanLogicalDevice().bindImageMemory(_vulkanImage, _vulkanMemory, 0);
    }
    catch (const vk::Error &error)
    {
        _device->vulkanLogicalDevice().destroyImage(_vulkanImage);
        _device->vulkanLogicalDevice().freeMemory(_vulkanMemory);
        throw TextureError(fmt::format("Can't bind the texture with his memory: {}", error.what()));
    }
}

VulkanTexture::~VulkanTexture()
{
    _device->vulkanLogicalDevice().destroyImage(_vulkanImage);
    _device->vulkanLogicalDevice().freeMemory(_vulkanMemory);
}

std::shared_ptr<TextureView> VulkanTexture::createTextureView(const TextureViewDescriptor &textureViewDescriptor) const
{
    return std::make_shared<VulkanTextureView>(_device, _vulkanImage, textureViewDescriptor);
}

} // namespace chronicle::graphics::internal::vulkan