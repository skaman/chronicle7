// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanBindGroupLayout.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace chronicle::graphics::internal::vulkan
{

VulkanBindGroupLayout::VulkanBindGroupLayout(std::shared_ptr<VulkanDevice> device,
                                             const BindGroupLayoutDescriptor &bindGroupLayoutDescriptor)
    : BindGroupLayout(bindGroupLayoutDescriptor), _device(device)
{
    const auto &entries = descriptor().entries;

    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(entries.size());
    for (auto i = 0; i < entries.size(); i++)
    {
        const auto &entry = entries[i];
        layoutBindings[i].binding = entry.binding;
        if (std::holds_alternative<BufferBindingLayout>(entry.layout))
        {
            const auto &layout = std::get<BufferBindingLayout>(entry.layout);
            layoutBindings[i].descriptorType = layout.hasDynamicOffset ? vk::DescriptorType::eUniformBufferDynamic
                                                                       : vk::DescriptorType::eUniformBuffer;
        }
        else if (std::holds_alternative<SamplerBindingLayout>(entry.layout))
        {
            layoutBindings[i].descriptorType = vk::DescriptorType::eSampler;
        }
        else if (std::holds_alternative<TextureBindingLayout>(entry.layout))
        {
            layoutBindings[i].descriptorType = vk::DescriptorType::eSampledImage;
        }
        else if (std::holds_alternative<StorageTextureBindingLayout>(entry.layout))
        {
            layoutBindings[i].descriptorType = vk::DescriptorType::eStorageImage;
        }
        else
        {
            throw BindGroupLayoutError(
                std::format("Invalid entry at binding {} for bind group layout {}", entry.binding, descriptor().name));
        }
    }

    /// Create descriptor set layout
    try
    {
        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, layoutBindings);
        _vulkanDescriptorSetLayout = _device->vulkanLogicalDevice().createDescriptorSetLayout(layoutInfo);
    }
    catch (const vk::Error &error)
    {
        throw BindGroupLayoutError(fmt::format("Can't create bind group layout: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eDescriptorSetLayout,
                               (uint64_t)(VkDescriptorSetLayout)_vulkanDescriptorSetLayout, descriptor().name);
#endif
}

VulkanBindGroupLayout::~VulkanBindGroupLayout()
{
    _device->vulkanLogicalDevice().destroyDescriptorSetLayout(_vulkanDescriptorSetLayout);
}

} // namespace chronicle::graphics::internal::vulkan