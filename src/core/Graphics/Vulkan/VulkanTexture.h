// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/Texture.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

/// @brief Vulkan implementation for Texture.
class VulkanTexture final : public Texture, private NonCopyable<VulkanTexture>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param textureDescriptor Texture descriptor.
    explicit VulkanTexture(std::shared_ptr<VulkanDevice> device, const TextureDescriptor &textureDescriptor);

    /// @brief Destructor.
    ~VulkanTexture() override;

    /// @brief Override for Texture::createTextureView.
    [[nodiscard]] std::shared_ptr<TextureView> createTextureView(
        const TextureViewDescriptor &textureViewDescriptor) const override;

  private:
    std::shared_ptr<VulkanDevice> _device{}; ///< Graphic device.
    vk::Image _vulkanImage{};                ///< Vulkan image.
    vk::DeviceMemory _vulkanMemory{};        ///< Vulkan device memory.
};

} // namespace chronicle::graphics::internal::vulkan