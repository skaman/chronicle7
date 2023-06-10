// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/TextureView.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

class VulkanTextureView final : public TextureView, private NonCopyable<VulkanTextureView>
{
  public:
    /// @brief Constructor.
    /// @param device Graphic device.
    /// @param image Vulkan image.
    /// @param textureViewDescriptor Texture view descriptor.
    explicit VulkanTextureView(std::shared_ptr<VulkanDevice> device, vk::Image image,
                               const TextureViewDescriptor &textureViewDescriptor);

    /// @brief Destructor.
    ~VulkanTextureView() override;

    /// @brief Returns the native vulkan image view.
    /// @return Vulkan image view.
    [[nodiscard]] vk::ImageView vulkanImageView() const
    {
        return _vulkanImageView;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{}; ///< Graphic device.
    vk::ImageView _vulkanImageView{};        ///< Vulkan image view.
};

} // namespace chronicle::graphics::internal::vulkan