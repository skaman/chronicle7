// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/Sampler.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

class VulkanSampler final : public Sampler, private NonCopyable<VulkanSampler>
{
  public:
    explicit VulkanSampler(std::shared_ptr<VulkanDevice> device, const SamplerCreateInfo &samplerCreateInfo);
    ~VulkanSampler() override;

    std::string_view name() const override
    {
        return _name;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{};
    std::string _name{};

    vk::Sampler _vulkanSampler{};
};

} // namespace chronicle::graphics::internal::vulkan