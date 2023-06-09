// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Graphics/Texture.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

class VulkanTexture final : public Texture, private NonCopyable<VulkanTexture>
{
  public:
    explicit VulkanTexture(std::shared_ptr<VulkanDevice> device, const TextureCreateInfo &textureCreateInfo);
    ~VulkanTexture() override;

    [[nodiscard]] std::shared_ptr<TextureView> createTextureView(
        const TextureViewCreateInfo &textureViewCreateInfo) const override;

    std::string_view name() const override
    {
        return _name;
    }

    uint32_t width() const override
    {
        return _width;
    }

    uint32_t height() const override
    {
        return _height;
    }

    uint32_t depth() const override
    {
        return _depth;
    }

    uint32_t arrayLayers() const override
    {
        return _arrayLayers;
    }

    uint32_t mipLevelCount() const override
    {
        return _mipLevelCount;
    }

    TextureViewDimension dimension() const override
    {
        return _dimension;
    }

    TextureFormat format() const override
    {
        return _format;
    }

    TextureUsageFlags usage() const override
    {
        return _usage;
    }

    TextureSampleCount sampleCount() const override
    {
        return _sampleCount;
    }

  private:
    std::shared_ptr<VulkanDevice> _device{};
    std::string _name{};

    uint32_t _width{};
    uint32_t _height{};
    uint32_t _depth{};
    uint32_t _arrayLayers{};
    uint32_t _mipLevelCount{};
    TextureViewDimension _dimension{};
    TextureFormat _format{};
    TextureUsageFlags _usage{};
    TextureSampleCount _sampleCount{};

    vk::Image _vulkanImage{};
    vk::DeviceMemory _vulkanMemory{};
};

} // namespace chronicle::graphics::internal::vulkan