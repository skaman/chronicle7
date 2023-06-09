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
    [[nodiscard]] std::shared_ptr<Buffer> createBuffer(const BufferCreateInfo &bufferCreateInfo) const override;

    void setDebugObjectName(vk::ObjectType objectType, uint64_t handle, const std::string &name) const;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    [[nodiscard]] vk::PhysicalDevice vulkanPhysicalDevice() const
    {
        return _physicalDevice;
    }

    [[nodiscard]] vk::Device vulkanLogicalDevice() const
    {
        return _logicalDevice;
    }

    [[nodiscard]] vk::CommandPool vulkanCommandPool() const
    {
        return _commandPool;
    }

    [[nodiscard]] vk::Queue vulkanGraphicsQueue() const
    {
        return _graphicsQueue;
    }

    [[nodiscard]] vk::Queue vulkanPresentQueue() const
    {
        return _presentQueue;
    }

  private:
    std::weak_ptr<VulkanDevice> _thisWeakPtr{};

    vk::PhysicalDevice _physicalDevice{};
    vk::Device _logicalDevice{};
    vk::Queue _graphicsQueue{};
    vk::Queue _presentQueue{};
    vk::CommandPool _commandPool{};

    friend class VulkanSystem;
};

} // namespace chronicle::graphics::internal::vulkan