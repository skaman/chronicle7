// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanTexture.h"

#include "VulkanDevice.h"

#include <magic_enum_flags.hpp>

namespace chronicle::graphics::internal::vulkan
{

inline vk::ImageType convertImageType(TextureDimension viewDimension)
{
    switch (viewDimension)
    {
    case TextureDimension::e1D:
        return vk::ImageType::e1D;
    case TextureDimension::e2D:
        return vk::ImageType::e2D;
    case TextureDimension::e3D:
        return vk::ImageType::e3D;
    }

    CHRLOG_ERROR("Unsupported texture dimension: {}", magic_enum::enum_name(viewDimension));

    return vk::ImageType::e2D;
}

inline vk::ImageViewType convertImageViewType(TextureViewDimension viewDimension)
{
    switch (viewDimension)
    {
    case TextureViewDimension::e1D:
        return vk::ImageViewType::e1D;
    case TextureViewDimension::e2D:
        return vk::ImageViewType::e2D;
    case TextureViewDimension::e3D:
        return vk::ImageViewType::e3D;
    }

    CHRLOG_ERROR("Unsupported texture dimension: {}", magic_enum::enum_name(viewDimension));

    return vk::ImageViewType::e2D;
}

const static std::unordered_map<TextureFormat, vk::Format> textureFormatMap = {
    // 8-bit formats
    {TextureFormat::eR8Unorm, vk::Format::eR8Unorm},
    {TextureFormat::eR8Snorm, vk::Format::eR8Snorm},
    {TextureFormat::eR8Uint, vk::Format::eR8Uint},
    {TextureFormat::eR8Sint, vk::Format::eR8Sint},

    // 16-bit formats
    {TextureFormat::eR16Uint, vk::Format::eR16Uint},
    {TextureFormat::eR16Sint, vk::Format::eR16Sint},
    {TextureFormat::eR16Sfloat, vk::Format::eR16Sfloat},
    {TextureFormat::eR16Unorm, vk::Format::eR16Unorm},
    {TextureFormat::eR16Snorm, vk::Format::eR16Snorm},
    {TextureFormat::eR8G8Uint, vk::Format::eR8G8Uint},
    {TextureFormat::eR8G8Sint, vk::Format::eR8G8Sint},

    // 32-bit formats
    {TextureFormat::eR32Uint, vk::Format::eR32Uint},
    {TextureFormat::eR32Sint, vk::Format::eR32Sint},
    {TextureFormat::eR32Sfloat, vk::Format::eR32Sfloat},
    {TextureFormat::eR16G16Uint, vk::Format::eR16G16Uint},
    {TextureFormat::eR16G16Sint, vk::Format::eR16G16Sint},
    {TextureFormat::eR16G16Sfloat, vk::Format::eR16G16Sfloat},
    {TextureFormat::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Unorm},
    {TextureFormat::eR8G8B8A8Srgb, vk::Format::eR8G8B8A8Srgb},
    {TextureFormat::eR8G8B8A8Snorm, vk::Format::eR8G8B8A8Snorm},
    {TextureFormat::eR8G8B8A8Uint, vk::Format::eR8G8B8A8Uint},
    {TextureFormat::eR8G8B8A8Sint, vk::Format::eR8G8B8A8Sint},
    {TextureFormat::eB8G8R8A8Unorm, vk::Format::eB8G8R8A8Unorm},
    {TextureFormat::eB8G8R8A8Srgb, vk::Format::eB8G8R8A8Srgb},

    // Packed 32-bit formats
    {TextureFormat::eE5B9G9R9UfloatPack32, vk::Format::eE5B9G9R9UfloatPack32},
    {TextureFormat::eA2R10G10B10UnormPack32, vk::Format::eA2R10G10B10UnormPack32},
    {TextureFormat::eB10G11R11UfloatPack32, vk::Format::eB10G11R11UfloatPack32},

    // 64-bit formats
    {TextureFormat::eR32G32Uint, vk::Format::eR32G32Uint},
    {TextureFormat::eR32G32Sint, vk::Format::eR32G32Sint},
    {TextureFormat::eR32G32Sfloat, vk::Format::eR32G32Sfloat},
    {TextureFormat::eR16G16B16A16Uint, vk::Format::eR16G16B16A16Uint},
    {TextureFormat::eR16G16B16A16Sint, vk::Format::eR16G16B16A16Sint},
    {TextureFormat::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat},

    // 128-bit formats
    {TextureFormat::eR32G32B32A32Uint, vk::Format::eR32G32B32A32Uint},
    {TextureFormat::eR32G32B32A32Sint, vk::Format::eR32G32B32A32Sint},
    {TextureFormat::eR32G32B32A32Sfloat, vk::Format::eR32G32B32A32Sfloat},

    // Depth/stencil formats
    {TextureFormat::eD32Sfloat, vk::Format::eD32Sfloat},
    {TextureFormat::eD32SfloatS8Uint, vk::Format::eD32SfloatS8Uint},
    {TextureFormat::eD24UnormS8Uint, vk::Format::eD24UnormS8Uint},

    // BC compressed formats
    {TextureFormat::eBc1RgbaUnormBlock, vk::Format::eBc1RgbaUnormBlock},
    {TextureFormat::eBc1RgbaSrgbBlock, vk::Format::eBc1RgbaSrgbBlock},
    {TextureFormat::eBc2UnormBlock, vk::Format::eBc2UnormBlock},
    {TextureFormat::eBc2SrgbBlock, vk::Format::eBc2SrgbBlock},
    {TextureFormat::eBc3UnormBlock, vk::Format::eBc3UnormBlock},
    {TextureFormat::eBc3SrgbBlock, vk::Format::eBc3SrgbBlock},
    {TextureFormat::eBc4UnormBlock, vk::Format::eBc4UnormBlock},
    {TextureFormat::eBc4SnormBlock, vk::Format::eBc4SnormBlock},
    {TextureFormat::eBc5UnormBlock, vk::Format::eBc5UnormBlock},
    {TextureFormat::eBc5SnormBlock, vk::Format::eBc5SnormBlock},
    {TextureFormat::eBc6HUfloatBlock, vk::Format::eBc6HUfloatBlock},
    {TextureFormat::eBc6HSfloatBlock, vk::Format::eBc6HSfloatBlock},
    {TextureFormat::eBc7UnormBlock, vk::Format::eBc7UnormBlock},
    {TextureFormat::eBc7SrgbBlock, vk::Format::eBc7SrgbBlock},

    // ETC2 compressed formats
    {TextureFormat::eEtc2R8G8B8UnormBlock, vk::Format::eEtc2R8G8B8UnormBlock},
    {TextureFormat::eEtc2R8G8B8SrgbBlock, vk::Format::eEtc2R8G8B8SrgbBlock},
    {TextureFormat::eEtc2R8G8B8A1UnormBlock, vk::Format::eEtc2R8G8B8A1UnormBlock},
    {TextureFormat::eEtc2R8G8B8A1SrgbBlock, vk::Format::eEtc2R8G8B8A1SrgbBlock},
    {TextureFormat::eEtc2R8G8B8A8UnormBlock, vk::Format::eEtc2R8G8B8A8UnormBlock},
    {TextureFormat::eEtc2R8G8B8A8SrgbBlock, vk::Format::eEtc2R8G8B8A8SrgbBlock},
    {TextureFormat::eEacR11UnormBlock, vk::Format::eEacR11UnormBlock},
    {TextureFormat::eEacR11SnormBlock, vk::Format::eEacR11SnormBlock},
    {TextureFormat::eEacR11G11UnormBlock, vk::Format::eEacR11G11UnormBlock},
    {TextureFormat::eEacR11G11SnormBlock, vk::Format::eEacR11G11SnormBlock},

    // ASTC compressed formats
    {TextureFormat::eAstc4x4UnormBlock, vk::Format::eAstc4x4UnormBlock},
    {TextureFormat::eAstc4x4SrgbBlock, vk::Format::eAstc4x4SrgbBlock},
    {TextureFormat::eAstc5x4UnormBlock, vk::Format::eAstc5x4UnormBlock},
    {TextureFormat::eAstc5x4SrgbBlock, vk::Format::eAstc5x4SrgbBlock},
    {TextureFormat::eAstc5x5UnormBlock, vk::Format::eAstc5x5UnormBlock},
    {TextureFormat::eAstc5x5SrgbBlock, vk::Format::eAstc5x5SrgbBlock},
    {TextureFormat::eAstc6x5UnormBlock, vk::Format::eAstc6x5UnormBlock},
    {TextureFormat::eAstc6x5SrgbBlock, vk::Format::eAstc6x5SrgbBlock},
    {TextureFormat::eAstc6x6UnormBlock, vk::Format::eAstc6x6UnormBlock},
    {TextureFormat::eAstc6x6SrgbBlock, vk::Format::eAstc6x6SrgbBlock},
    {TextureFormat::eAstc8x5UnormBlock, vk::Format::eAstc8x5UnormBlock},
    {TextureFormat::eAstc8x5SrgbBlock, vk::Format::eAstc8x5SrgbBlock},
    {TextureFormat::eAstc8x6UnormBlock, vk::Format::eAstc8x6UnormBlock},
    {TextureFormat::eAstc8x6SrgbBlock, vk::Format::eAstc8x6SrgbBlock},
    {TextureFormat::eAstc8x8UnormBlock, vk::Format::eAstc8x8UnormBlock},
    {TextureFormat::eAstc8x8SrgbBlock, vk::Format::eAstc8x8SrgbBlock},
    {TextureFormat::eAstc10x5UnormBlock, vk::Format::eAstc10x5UnormBlock},
    {TextureFormat::eAstc10x5SrgbBlock, vk::Format::eAstc10x5SrgbBlock},
    {TextureFormat::eAstc10x6UnormBlock, vk::Format::eAstc10x6UnormBlock},
    {TextureFormat::eAstc10x6SrgbBlock, vk::Format::eAstc10x6SrgbBlock},
    {TextureFormat::eAstc10x8UnormBlock, vk::Format::eAstc10x8UnormBlock},
    {TextureFormat::eAstc10x8SrgbBlock, vk::Format::eAstc10x8SrgbBlock},
    {TextureFormat::eAstc10x10UnormBlock, vk::Format::eAstc10x10UnormBlock},
    {TextureFormat::eAstc10x10SrgbBlock, vk::Format::eAstc10x10SrgbBlock},
    {TextureFormat::eAstc12x10UnormBlock, vk::Format::eAstc12x10UnormBlock},
    {TextureFormat::eAstc12x10SrgbBlock, vk::Format::eAstc12x10SrgbBlock},
    {TextureFormat::eAstc12x12UnormBlock, vk::Format::eAstc12x12UnormBlock},
    {TextureFormat::eAstc12x12SrgbBlock, vk::Format::eAstc12x12SrgbBlock}};

inline vk::Format convertFormat(TextureFormat format)
{
    return textureFormatMap.at(format);
}

inline vk::ImageUsageFlags convertUsage(TextureUsageFlags flags)
{
    vk::ImageUsageFlags result{};
    if (magic_enum::enum_flags_test_any(flags, TextureUsageFlags::eCopySrc))
    {
        result |= vk::ImageUsageFlagBits::eTransferSrc;
    }
    if (magic_enum::enum_flags_test_any(flags, TextureUsageFlags::eCopyDst))
    {
        result |= vk::ImageUsageFlagBits::eTransferDst;
    }
    if (magic_enum::enum_flags_test_any(flags, TextureUsageFlags::eTextureBinding))
    {
        result |= vk::ImageUsageFlagBits::eSampled;
    }
    if (magic_enum::enum_flags_test_any(flags, TextureUsageFlags::eStorageBinding))
    {
        result |= vk::ImageUsageFlagBits::eStorage;
    }
    if (magic_enum::enum_flags_test_any(flags, TextureUsageFlags::eColorAttachment))
    {
        result |= vk::ImageUsageFlagBits::eColorAttachment;
    }
    if (magic_enum::enum_flags_test_any(flags, TextureUsageFlags::eDepthAttachment))
    {
        result |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    }
    return result;
}

inline vk::SampleCountFlagBits convertSamples(TextureSampleCount samples)
{
    switch (samples)
    {
    case TextureSampleCount::e1:
        return vk::SampleCountFlagBits::e1;
    case TextureSampleCount::e2:
        return vk::SampleCountFlagBits::e2;
    case TextureSampleCount::e4:
        return vk::SampleCountFlagBits::e4;
    case TextureSampleCount::e8:
        return vk::SampleCountFlagBits::e8;
    case TextureSampleCount::e16:
        return vk::SampleCountFlagBits::e16;
    case TextureSampleCount::e32:
        return vk::SampleCountFlagBits::e32;
    case TextureSampleCount::e64:
        return vk::SampleCountFlagBits::e64;
    }

    CHRLOG_ERROR("Unsupported sample count: {}", magic_enum::enum_name(samples));

    return vk::SampleCountFlagBits::e1;
}

inline vk::ImageAspectFlags convertAspect(TextureAspect aspect)
{
    switch (aspect)
    {
    case TextureAspect::eColor:
        return vk::ImageAspectFlagBits::eColor;
    case TextureAspect::eDepthOnly:
        return vk::ImageAspectFlagBits::eDepth;
    case TextureAspect::eStencilOnly:
        return vk::ImageAspectFlagBits::eStencil;
    case TextureAspect::eDepthAndStencil:
        return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    }

    CHRLOG_ERROR("Unsupported texture aspect: {}", magic_enum::enum_name(aspect));

    return vk::ImageAspectFlagBits::eColor;
}

inline vk::SamplerAddressMode convertSamplerAddressMode(AddressMode addressMode)
{
    switch (addressMode)
    {
    case AddressMode::eClampToEdge:
        return vk::SamplerAddressMode::eClampToEdge;
    case AddressMode::eRepeat:
        return vk::SamplerAddressMode::eRepeat;
    case AddressMode::eMirrorRepeat:
        return vk::SamplerAddressMode::eMirroredRepeat;
    }

    CHRLOG_ERROR("Unsupported address mode: {}", magic_enum::enum_name(addressMode));

    return vk::SamplerAddressMode::eClampToEdge;
}

inline vk::Filter convertFilter(FilterMode filterMode)
{
    switch (filterMode)
    {
    case FilterMode::eNearest:
        return vk::Filter::eNearest;
    case FilterMode::eLinear:
        return vk::Filter::eLinear;
    }

    CHRLOG_ERROR("Unsupported filter mode: {}", magic_enum::enum_name(filterMode));

    return vk::Filter::eNearest;
}

inline vk::SamplerMipmapMode convertSamplerMipmap(MipmapFilterMode mipmapFilterMode)
{
    switch (mipmapFilterMode)
    {
    case MipmapFilterMode::eNearest:
        return vk::SamplerMipmapMode::eNearest;
    case MipmapFilterMode::eLinear:
        return vk::SamplerMipmapMode::eLinear;
    }

    CHRLOG_ERROR("Unsupported mipmap filter mode: {}", magic_enum::enum_name(mipmapFilterMode));

    return vk::SamplerMipmapMode::eNearest;
}

inline vk::CompareOp converCompareOp(CompareFunction compareFunction)
{
    switch (compareFunction)
    {
    case CompareFunction::eNever:
        return vk::CompareOp::eNever;
    case CompareFunction::eLess:
        return vk::CompareOp::eLess;
    case CompareFunction::eEqual:
        return vk::CompareOp::eEqual;
    case CompareFunction::eLessEqual:
        return vk::CompareOp::eLessOrEqual;
    case CompareFunction::eGreater:
        return vk::CompareOp::eGreater;
    case CompareFunction::eNotEqual:
        return vk::CompareOp::eNotEqual;
    case CompareFunction::eGreaterEqual:
        return vk::CompareOp::eGreaterOrEqual;
    case CompareFunction::eAlways:
        return vk::CompareOp::eAlways;
    }

    CHRLOG_ERROR("Unsupported compare function: {}", magic_enum::enum_name(compareFunction));

    return vk::CompareOp::eNever;
}

} // namespace chronicle::graphics::internal::vulkan