// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/BindGroupLayout.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

class VulkanBindGroupLayout final : public BindGroupLayout, private NonCopyable<VulkanBindGroupLayout>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param bindGroupLayoutDescriptor Bind group layout descriptor.
    explicit VulkanBindGroupLayout(std::shared_ptr<VulkanDevice> device,
                                   const BindGroupLayoutDescriptor &bindGroupLayoutDescriptor);

    /// @brief Destructor.
    ~VulkanBindGroupLayout() override;

    /// @brief Returns the vulkan descriptor set layout.
    /// @return Descriptor set layout.
    [[nodiscard]] vk::DescriptorSetLayout vulkanDescriptorSetLayout() const
    {
        return _vulkanDescriptorSetLayout;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{};              ///< Graphic device.
    vk::DescriptorSetLayout _vulkanDescriptorSetLayout{}; ///< Vulkan descriptor set layout.
};

} // namespace chronicle::graphics::internal::vulkan