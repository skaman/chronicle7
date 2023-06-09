// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTextureView.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

VulkanTextureView::VulkanTextureView(std::shared_ptr<VulkanDevice> device, vk::Image image,
                                     const TextureViewCreateInfo &textureCreateInfo)
    : _device(device), _name(textureCreateInfo.name), _format(textureCreateInfo.format),
      _dimension(textureCreateInfo.dimension), _aspect(textureCreateInfo.aspect),
      _baseMipLevel(textureCreateInfo.baseMipLevel), _mipLevelCount(textureCreateInfo.mipLevelCount),
      _baseArrayLayer(textureCreateInfo.baseArrayLayer), _arrayLayerCount(textureCreateInfo.arrayLayerCount)
{
    vk::ImageSubresourceRange subresourceRange(convertAspect(_aspect), _baseMipLevel, _mipLevelCount, _baseArrayLayer,
                                               _arrayLayerCount);

    vk::ImageViewCreateInfo viewInfo({}, image, convertImageViewType(_dimension), convertFormat(_format), {},
                                     subresourceRange);
    _vulkanImageView = _device->vulkanLogicalDevice().createImageView(viewInfo);
}

VulkanTextureView::~VulkanTextureView()
{
    _device->vulkanLogicalDevice().destroyImageView(_vulkanImageView);
}

} // namespace chronicle::graphics::internal::vulkan