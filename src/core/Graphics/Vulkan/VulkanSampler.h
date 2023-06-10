// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/Sampler.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

/// @brief Vulkan implementation for Sampler.
class VulkanSampler final : public Sampler, private NonCopyable<VulkanSampler>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param samplerDescriptor Sampler descriptor.
    explicit VulkanSampler(std::shared_ptr<VulkanDevice> device, const SamplerDescriptor &samplerDescriptor);

    /// @brief Destructor.
    ~VulkanSampler() override;

    /// @brief Returns the native vulkan sampler.
    /// @return Vulkan sampler.
    [[nodiscard]] vk::Sampler vulkanSampler() const
    {
        return _vulkanSampler;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{}; ///< Graphic device.
    vk::Sampler _vulkanSampler{};            ///< Vulkan sampler.
};

} // namespace chronicle::graphics::internal::vulkan