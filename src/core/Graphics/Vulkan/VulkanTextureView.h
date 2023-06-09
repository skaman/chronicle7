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
    explicit VulkanTextureView(std::shared_ptr<VulkanDevice> device, vk::Image image,
                               const TextureViewCreateInfo &textureCreateInfo);
    ~VulkanTextureView() override;

    std::string_view name() const override
    {
        return _name;
    }

    TextureFormat format() const override
    {
        return _format;
    }

    TextureViewDimension dimension() const override
    {
        return _dimension;
    }

    TextureAspect aspect() const override
    {
        return _aspect;
    }

    uint32_t baseMipLevel() const override
    {
        return _baseMipLevel;
    }

    uint32_t mipLevelCount() const override
    {
        return _mipLevelCount;
    }

    uint32_t baseArrayLayer() const override
    {
        return _baseArrayLayer;
    }

    uint32_t arrayLayerCount() const override
    {
        return _arrayLayerCount;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{};
    std::string _name{};

    TextureFormat _format{};
    TextureViewDimension _dimension{TextureViewDimension::e2D};
    TextureAspect _aspect{};
    uint32_t _baseMipLevel{};
    uint32_t _mipLevelCount{};
    uint32_t _baseArrayLayer{};
    uint32_t _arrayLayerCount{};

    vk::ImageView _vulkanImageView{};
};

} // namespace chronicle::graphics::internal::vulkan