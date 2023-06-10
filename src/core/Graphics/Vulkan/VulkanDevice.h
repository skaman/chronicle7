// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Graphics/Device.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

struct VulkanQueueFamilyIndices;

class VulkanDevice final : public Device, private NonCopyable<VulkanDevice>
{
  public:
    explicit VulkanDevice(vk::PhysicalDevice physicalDevice, const VulkanQueueFamilyIndices &indices);
    ~VulkanDevice() override;

    [[nodiscard]] std::shared_ptr<CommandEncoder> createCommandEncoder(
        const CommandEncoderCreateInfo &commandEncoderCreateInfo) const override;
    [[nodiscard]] std::shared_ptr<Buffer> createBuffer(const BufferDescriptor &bufferDescriptor) const override;
    [[nodiscard]] std::shared_ptr<Sampler> createSampler(const SamplerDescriptor &samplerDescriptor) const override;

    void setDebugObjectName(vk::ObjectType objectType, uint64_t handle, const std::string &name) const;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    [[nodiscard]] vk::PhysicalDevice vulkanPhysicalDevice() const
    {
        return _vulkanPhysicalDevice;
    }

    [[nodiscard]] vk::Device vulkanLogicalDevice() const
    {
        return _vulkanLogicalDevice;
    }

    [[nodiscard]] vk::CommandPool vulkanCommandPool() const
    {
        return _vulkanCommandPool;
    }

    [[nodiscard]] vk::DescriptorPool vulkanDescriptorPool() const
    {
        return _vulkanDescriptorPool;
    }

    [[nodiscard]] vk::Queue vulkanGraphicsQueue() const
    {
        return _vulkanGraphicsQueue;
    }

    [[nodiscard]] vk::Queue vulkanPresentQueue() const
    {
        return _vulkanPresentQueue;
    }

  private:
    std::weak_ptr<VulkanDevice> _thisWeakPtr{};

    vk::PhysicalDevice _vulkanPhysicalDevice{};
    vk::Device _vulkanLogicalDevice{};
    vk::Queue _vulkanGraphicsQueue{};
    vk::Queue _vulkanPresentQueue{};
    vk::CommandPool _vulkanCommandPool{};
    vk::DescriptorPool _vulkanDescriptorPool{};

    friend class VulkanSystem;
};

} // namespace chronicle::graphics::internal::vulkan