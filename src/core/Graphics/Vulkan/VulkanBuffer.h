// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Graphics/Buffer.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

/// @brief Vulkan implementation for Buffer.
class VulkanBuffer final : public Buffer, private NonCopyable<VulkanBuffer>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param bufferDescriptor Buffer descriptor.
    explicit VulkanBuffer(std::shared_ptr<VulkanDevice> device, const BufferDescriptor &bufferDescriptor);

    /// @brief Destructor.
    ~VulkanBuffer() override;

    /// @brief Override for Buffer::map.
    void map() override;

    /// @brief Override for Buffer::unmap.
    void unmap() override;

    /// @brief Override for Buffer::mappedRange().
    [[nodiscard]] std::span<uint8_t> mappedRange() const override
    {
        return _memoryMap;
    }

    /// @brief Override for Buffer::mappedRange(uint64_t).
    [[nodiscard]] std::span<uint8_t> mappedRange(uint64_t offset) const override
    {
        return mappedRange(offset, _memoryMap.size() - offset);
    }

    /// @brief Override for Buffer::mappedRange(uint64_t,uint64_t).
    [[nodiscard]] std::span<uint8_t> mappedRange(uint64_t offset, uint64_t size) const override
    {
        return _memoryMap.subspan(offset, size);
    }

    /// @brief Override for Buffer::isMapped.
    [[nodiscard]] bool isMapped() const override
    {
        return _isMapped;
    }

    /// @brief Returns the native vulkan buffer.
    /// @return Vulkan buffer.
    [[nodiscard]] vk::Buffer vulkanBuffer() const
    {
        return _vulkanBuffer;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{}; ///< Graphic device.
    bool _isHostVisible{};                   ///< The buffer is host visible.
    bool _isMapped{};                        ///< The buffer is mapped.
    vk::Buffer _vulkanBuffer{};              ///< Vulkan buffer.
    vk::DeviceMemory _vulkanMemory{};        ///< Vulkan device memory.
    std::span<uint8_t> _memoryMap{};         ///< Mapped memory.
};

} // namespace chronicle::graphics::internal::vulkan