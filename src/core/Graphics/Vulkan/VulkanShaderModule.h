// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/ShaderModule.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

/// @brief Vulkan implementation for ShaderModule.
class VulkanShaderModule final : public ShaderModule, private NonCopyable<VulkanShaderModule>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param shaderModuleDescriptor Shader module descriptor.
    explicit VulkanShaderModule(std::shared_ptr<VulkanDevice> device,
                                const ShaderModuleDescriptor &shaderModuleDescriptor);

    /// @brief Destructor.
    ~VulkanShaderModule() override;

    /// @brief Returns the native vulkan shader module.
    /// @return Vulkan shader module.
    [[nodiscard]] vk::ShaderModule vulkanShaderModule() const
    {
        return _vulkanShaderModule;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{}; ///< Graphic device.
    vk::ShaderModule _vulkanShaderModule{};  ///< Vulkan shader module.
};

} // namespace chronicle::graphics::internal::vulkan