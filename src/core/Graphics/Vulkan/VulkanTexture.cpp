// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTexture.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanTextureView.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

VulkanTexture::VulkanTexture(std::shared_ptr<VulkanDevice> device, const TextureCreateInfo &textureCreateInfo)
    : _device(device), _name(textureCreateInfo.name), _width(textureCreateInfo.width),
      _height(textureCreateInfo.height), _depth(textureCreateInfo.depth), _arrayLayers(textureCreateInfo.arrayLayers),
      _mipLevelCount(textureCreateInfo.mipLevelCount), _dimension(textureCreateInfo.dimension),
      _format(textureCreateInfo.format), _usage(textureCreateInfo.usage), _sampleCount(textureCreateInfo.sampleCount)
{
    vk::Extent3D imageExtent(textureCreateInfo.width, textureCreateInfo.height, textureCreateInfo.depth);

    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(convertImageType(textureCreateInfo.dimension));
    imageCreateInfo.setExtent(imageExtent);
    imageCreateInfo.setMipLevels(textureCreateInfo.mipLevelCount);
    imageCreateInfo.setArrayLayers(textureCreateInfo.arrayLayers);
    imageCreateInfo.setFormat(convertFormat(textureCreateInfo.format));
    imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setUsage(convertUsage(textureCreateInfo.usage));
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    imageCreateInfo.setSamples(convertSamples(textureCreateInfo.sampleCount));

    auto logicalDevice = _device->vulkanLogicalDevice();
    _vulkanImage = logicalDevice.createImage(imageCreateInfo);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eImage, (uint64_t)(VkImage)_vulkanImage, _name);
#endif

    // allocate memory
    const auto memRequirements = logicalDevice.getImageMemoryRequirements(_vulkanImage);
    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(
        _device->findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
    _vulkanMemory = logicalDevice.allocateMemory(allocInfo, nullptr);

    // bind image memory
    logicalDevice.bindImageMemory(_vulkanImage, _vulkanMemory, 0);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eDeviceMemory, (uint64_t)(VkDeviceMemory)_vulkanMemory, _name);
#endif
}

VulkanTexture::~VulkanTexture()
{
    auto logcalDevice = _device->vulkanLogicalDevice();
    logcalDevice.destroyImage(_vulkanImage);
    logcalDevice.freeMemory(_vulkanMemory);
}

std::shared_ptr<TextureView> VulkanTexture::createTextureView(const TextureViewCreateInfo &textureViewCreateInfo) const
{
    return std::make_shared<VulkanTextureView>(_device, _vulkanImage, textureViewCreateInfo);
}

} // namespace chronicle::graphics::internal::vulkan