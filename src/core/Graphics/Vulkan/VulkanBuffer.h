// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Graphics/Buffer.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

class VulkanBuffer : public Buffer, private NonCopyable<VulkanBuffer>
{
  public:
    explicit VulkanBuffer(std::shared_ptr<VulkanDevice> device, const BufferCreateInfo &bufferCreateInfo);
    ~VulkanBuffer();

    [[nodiscard]] std::span<uint8_t> data() const override
    {
        return _memoryMap;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{};
    std::string _name{};

    vk::Buffer _buffer{};
    vk::DeviceMemory _memory{};
    std::span<uint8_t> _memoryMap{};

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
};

} // namespace chronicle::graphics::internal::vulkan