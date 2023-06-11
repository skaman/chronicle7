// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Graphics/PipelineLayout.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

/// @brief Vulkan implementation for PipelineLayout.
class VulkanPipelineLayout final : public PipelineLayout, private NonCopyable<VulkanPipelineLayout>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param pipelineLayoutDescriptor Pipeline layout descriptor.
    explicit VulkanPipelineLayout(std::shared_ptr<VulkanDevice> device,
                                  const PipelineLayoutDescriptor &pipelineLayoutDescriptor);

    /// @brief Destructor.
    ~VulkanPipelineLayout() override;

    /// @brief Returns the native vulkan descriptor sets layout.
    /// @return Vulkan descriptor sets layout.
    [[nodiscard]] std::vector<vk::DescriptorSetLayout> vulkanDescriptorSetLayouts() const
    {
        return _descriptorSetLayouts;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{};                      ///< Graphic device.
    std::vector<vk::DescriptorSetLayout> _descriptorSetLayouts{}; ///< Vulkan descriptor sets layout.
};

} // namespace chronicle::graphics::internal::vulkan