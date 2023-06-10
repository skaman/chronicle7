// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTextureView.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

VulkanTextureView::VulkanTextureView(std::shared_ptr<VulkanDevice> device, vk::Image image,
                                     const TextureViewDescriptor &textureViewDescriptor)
    : TextureView(textureViewDescriptor), _device(device)
{
    // Create texture view.
    try
    {
        vk::ImageSubresourceRange subresourceRange(convertAspect(descriptor().aspect), descriptor().baseMipLevel,
                                                   descriptor().mipLevelCount, descriptor().baseArrayLayer,
                                                   descriptor().arrayLayerCount);

        vk::ImageViewCreateInfo viewInfo({}, image, convertImageViewType(descriptor().dimension),
                                         convertFormat(descriptor().format), {}, subresourceRange);
        _vulkanImageView = _device->vulkanLogicalDevice().createImageView(viewInfo);
    }
    catch (const vk::Error &error)
    {
        throw TextureViewError(fmt::format("Can't create texture view: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eImageView, (uint64_t)(VkImageView)_vulkanImageView, descriptor().name);
#endif
}

VulkanTextureView::~VulkanTextureView()
{
    _device->vulkanLogicalDevice().destroyImageView(_vulkanImageView);
}

} // namespace chronicle::graphics::internal::vulkan