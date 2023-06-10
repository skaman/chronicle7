// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/BindGroup.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

/// @brief Vulkan implementation for BindGroup.
class VulkanBindGroup final : public BindGroup, private NonCopyable<VulkanBindGroup>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param bindGroupDescriptor Binding group descriptor.
    explicit VulkanBindGroup(std::shared_ptr<VulkanDevice> device, const BindGroupDescriptor &bindGroupDescriptor);

    /// @brief Destructor.
    ~VulkanBindGroup() override;

  private:
    std::shared_ptr<VulkanDevice> _device{};  ///< Graphic device.
    vk::DescriptorSet _vulkanDescriptorSet{}; ///< Vulkan descriptor set.

    /// @brief Cleanup resources.
    void cleanup() const;
};

} // namespace chronicle::graphics::internal::vulkan