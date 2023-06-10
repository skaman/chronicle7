// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanBindGroup.h"

#include "VulkanBindGroupLayout.h"
#include "VulkanBuffer.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanSampler.h"
#include "VulkanTextureView.h"

namespace chronicle::graphics::internal::vulkan
{

VulkanBindGroup::VulkanBindGroup(std::shared_ptr<VulkanDevice> device, const BindGroupDescriptor &bindGroupDescriptor)
    : BindGroup(bindGroupDescriptor), _device(device)
{
    const auto *bindGroupLayout = static_cast<VulkanBindGroupLayout *>(bindGroupDescriptor.layout.get());
    auto descriptorSetLayout = bindGroupLayout->vulkanDescriptorSetLayout();

    // temporary put the entries into a map
    std::unordered_map<uint32_t, BindGroupLayoutEntry> layoutEntriesMap{};
    for (const auto &layoutEntry : descriptor().layout->descriptor().entries)
    {
        layoutEntriesMap[layoutEntry.binding] = layoutEntry;
    }

    // Allocate the descriptor set.
    try
    {
        vk::DescriptorSetAllocateInfo allocInfo(_device->vulkanDescriptorPool(), descriptorSetLayout);
        _vulkanDescriptorSet = _device->vulkanLogicalDevice().allocateDescriptorSets(allocInfo)[0];
    }
    catch (const vk::Error &error)
    {
        throw BindGroupError(fmt::format("Can't allocate the bind group: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eDescriptorSet, (uint64_t)(VkDescriptorSet)_vulkanDescriptorSet,
                               descriptor().name);
#endif

    // create the descriptors writes
    std::vector<vk::WriteDescriptorSet> descriptorWrites(descriptor().entries.size());
    for (uint32_t i = 0; i < descriptor().entries.size(); i++)
    {
        const auto &entry = descriptor().entries[i];

        if (!layoutEntriesMap.contains(entry.binding))
        {
            cleanup();
            throw BindGroupError(fmt::format("Binding {} not found in related bind group layout", entry.binding));
        }
        const auto &layoutEntry = layoutEntriesMap.at(entry.binding);

        if (std::holds_alternative<BufferBindingLayout>(layoutEntry.layout))
        {
            if (const auto *binding = std::get_if<BufferBinding>(&entry.resource))
            {
                const auto &buffer = static_cast<VulkanBuffer *>(binding->buffer.get());
                vk::DescriptorBufferInfo bufferInfo(buffer->vulkanBuffer(), binding->offset, binding->size);

                vk::WriteDescriptorSet descriptorWrite{};
                descriptorWrite.setDstSet(_vulkanDescriptorSet);
                descriptorWrite.setDstBinding(layoutEntry.binding);
                descriptorWrite.setDstArrayElement(0);
                descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
                descriptorWrite.setDescriptorCount(1);
                descriptorWrite.setBufferInfo(bufferInfo);
                descriptorWrites[i] = descriptorWrite;
            }
            else
            {
                cleanup();
                throw BindGroupError(fmt::format("Binding {} mismatch with related bind group layout", entry.binding));
            }
        }
        else if (std::holds_alternative<SamplerBindingLayout>(layoutEntry.layout))
        {
            if (const auto *binding = std::get_if<std::shared_ptr<Sampler>>(&entry.resource))
            {
                vk::DescriptorImageInfo imageInfo{};
                imageInfo.setSampler(static_cast<VulkanSampler *>(binding->get())->vulkanSampler());

                vk::WriteDescriptorSet descriptorWrite{};
                descriptorWrite.setDstSet(_vulkanDescriptorSet);
                descriptorWrite.setDstBinding(layoutEntry.binding);
                descriptorWrite.setDstArrayElement(0);
                descriptorWrite.setDescriptorType(vk::DescriptorType::eSampler);
                descriptorWrite.setDescriptorCount(1);
                descriptorWrite.setImageInfo(imageInfo);
                descriptorWrites[i] = descriptorWrite;
            }
            else
            {
                cleanup();
                throw BindGroupError(fmt::format("Binding {} mismatch with related bind group layout", entry.binding));
            }
        }
        else if (std::holds_alternative<TextureBindingLayout>(layoutEntry.layout))
        {
            if (const auto *binding = std::get_if<std::shared_ptr<TextureView>>(&entry.resource))
            {
                const auto *textureView = static_cast<VulkanTextureView *>(binding->get());
                vk::DescriptorImageInfo imageInfo{};
                imageInfo.setImageView(textureView->vulkanImageView());
                imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

                vk::WriteDescriptorSet descriptorWrite{};
                descriptorWrite.setDstSet(_vulkanDescriptorSet);
                descriptorWrite.setDstBinding(layoutEntry.binding);
                descriptorWrite.setDstArrayElement(0);
                descriptorWrite.setDescriptorType(vk::DescriptorType::eSampledImage);
                descriptorWrite.setDescriptorCount(1);
                descriptorWrite.setImageInfo(imageInfo);
                descriptorWrites[i] = descriptorWrite;
            }
            else
            {
                cleanup();
                throw BindGroupError(fmt::format("Binding {} mismatch with related bind group layout", entry.binding));
            }
        }
        else if (std::holds_alternative<StorageTextureBindingLayout>(layoutEntry.layout))
        {
            if (const auto *binding = std::get_if<std::shared_ptr<TextureView>>(&entry.resource))
            {
                const auto *textureView = static_cast<VulkanTextureView *>(binding->get());
                vk::DescriptorImageInfo imageInfo{};
                imageInfo.setImageView(textureView->vulkanImageView());

                vk::WriteDescriptorSet descriptorWrite{};
                descriptorWrite.setDstSet(_vulkanDescriptorSet);
                descriptorWrite.setDstBinding(layoutEntry.binding);
                descriptorWrite.setDstArrayElement(0);
                descriptorWrite.setDescriptorType(vk::DescriptorType::eStorageImage);
                descriptorWrite.setDescriptorCount(1);
                descriptorWrite.setImageInfo(imageInfo);
                descriptorWrites[i] = descriptorWrite;
            }
            else
            {
                cleanup();
                throw BindGroupError(fmt::format("Binding {} mismatch with related bind group layout", entry.binding));
            }
        }
    }

    // Update the descriptor sets.
    _device->vulkanLogicalDevice().updateDescriptorSets(descriptorWrites, nullptr);
}

VulkanBindGroup::~VulkanBindGroup()
{
    cleanup();
}

void VulkanBindGroup::cleanup() const
{
    if (_vulkanDescriptorSet)
    {
        _device->vulkanLogicalDevice().freeDescriptorSets(_device->vulkanDescriptorPool(), _vulkanDescriptorSet);
    }
}

} // namespace chronicle::graphics::internal::vulkan