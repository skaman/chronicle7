// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <stdexcept>

#include <magic_enum.hpp>

namespace chronicle::graphics
{

enum class TextureFormat : uint32_t
{
    // 8-bit formats
    eR8Unorm,
    eR8Snorm,
    eR8Uint,
    eR8Sint,

    // 16-bit formats
    eR16Uint,
    eR16Sint,
    eR16Sfloat,
    eR16Unorm,
    eR16Snorm,
    eR8G8Uint,
    eR8G8Sint,

    // 32-bit formats
    eR32Uint,
    eR32Sint,
    eR32Sfloat,
    eR16G16Uint,
    eR16G16Sint,
    eR16G16Sfloat,
    eR8G8B8A8Unorm,
    eR8G8B8A8Srgb,
    eR8G8B8A8Snorm,
    eR8G8B8A8Uint,
    eR8G8B8A8Sint,
    eB8G8R8A8Unorm,
    eB8G8R8A8Srgb,

    // Packed 32-bit formats
    eE5B9G9R9UfloatPack32,
    eA2R10G10B10UnormPack32,
    eB10G11R11UfloatPack32,

    // 64-bit formats
    eR32G32Uint,
    eR32G32Sint,
    eR32G32Sfloat,
    eR16G16B16A16Uint,
    eR16G16B16A16Sint,
    eR16G16B16A16Sfloat,

    // 128-bit formats
    eR32G32B32A32Uint,
    eR32G32B32A32Sint,
    eR32G32B32A32Sfloat,

    // Depth/stencil formats
    eD32Sfloat,
    eD32SfloatS8Uint,
    eD24UnormS8Uint,

    // BC compressed formats
    eBc1RgbaUnormBlock,
    eBc1RgbaSrgbBlock,
    eBc2UnormBlock,
    eBc2SrgbBlock,
    eBc3UnormBlock,
    eBc3SrgbBlock,
    eBc4UnormBlock,
    eBc4SnormBlock,
    eBc5UnormBlock,
    eBc5SnormBlock,
    eBc6HUfloatBlock,
    eBc6HSfloatBlock,
    eBc7UnormBlock,
    eBc7SrgbBlock,

    // ETC2 compressed formats
    eEtc2R8G8B8UnormBlock,
    eEtc2R8G8B8SrgbBlock,
    eEtc2R8G8B8A1UnormBlock,
    eEtc2R8G8B8A1SrgbBlock,
    eEtc2R8G8B8A8UnormBlock,
    eEtc2R8G8B8A8SrgbBlock,
    eEacR11UnormBlock,
    eEacR11SnormBlock,
    eEacR11G11UnormBlock,
    eEacR11G11SnormBlock,

    // ASTC compressed formats
    eAstc4x4UnormBlock,
    eAstc4x4SrgbBlock,
    eAstc5x4UnormBlock,
    eAstc5x4SrgbBlock,
    eAstc5x5UnormBlock,
    eAstc5x5SrgbBlock,
    eAstc6x5UnormBlock,
    eAstc6x5SrgbBlock,
    eAstc6x6UnormBlock,
    eAstc6x6SrgbBlock,
    eAstc8x5UnormBlock,
    eAstc8x5SrgbBlock,
    eAstc8x6UnormBlock,
    eAstc8x6SrgbBlock,
    eAstc8x8UnormBlock,
    eAstc8x8SrgbBlock,
    eAstc10x5UnormBlock,
    eAstc10x5SrgbBlock,
    eAstc10x6UnormBlock,
    eAstc10x6SrgbBlock,
    eAstc10x8UnormBlock,
    eAstc10x8SrgbBlock,
    eAstc10x10UnormBlock,
    eAstc10x10SrgbBlock,
    eAstc12x10UnormBlock,
    eAstc12x10SrgbBlock,
    eAstc12x12UnormBlock,
    eAstc12x12SrgbBlock,
};

enum class TextureViewDimension : uint32_t
{
    /// @brief The texture is viewed as a 1-dimensional image.
    e1D,

    /// @brief The texture is viewed as a 1-dimensional image.
    e2D,

    /// @brief The texture is viewed as a 3-dimensional image.
    e3D,
};

enum class TextureUsageFlags : uint32_t
{
    eCopySrc = 1 << 2,
    eCopyDst = 1 << 3,
    eTextureBinding = 1 << 4,
    eStorageBinding = 1 << 5,
    eColorAttachment = 1 << 6,
    eDepthAttachment = 1 << 7
};

enum class TextureSampleCount : uint32_t
{
    e1,
    e2,
    e4,
    e8,
    e16,
    e32,
    e64
};

enum class TextureAspect : uint32_t
{
    eColor,
    eStencilOnly,
    eDepthOnly,
    eDepthAndStencil
};

enum class AddressMode : uint32_t
{
    eClampToEdge,
    eRepeat,
    eMirrorRepeat
};

enum class FilterMode : uint32_t
{
    eNearest,
    eLinear
};

enum class MipmapFilterMode : uint32_t
{
    eNearest,
    eLinear
};

enum class CompareFunction : uint32_t
{
    eNever,
    eLess,
    eEqual,
    eLessEqual,
    eGreater,
    eNotEqual,
    eGreaterEqual,
    eAlways
};

/// @brief Informations used to initialize the graphic system.
struct SystemInitInfo
{
    std::string applicationName{"Chronicle"}; /// Application name.
    bool enableDebug{};                       /// Enable graphic debug informations.
};

/// @brief Informations used to request a graphic device.
struct RequestDeviceInfo
{
    void *hwnd{}; /// Window handler that the device should need to be compatible to.
};

class GraphicsError : public std::runtime_error
{
  public:
    using runtime_error::runtime_error;

    explicit GraphicsError(const std::string &message) : runtime_error(message.c_str())
    {
    }
};

} // namespace chronicle::graphics

template <> struct magic_enum::customize::enum_range<chronicle::graphics::TextureUsageFlags>
{
    static constexpr bool is_flags = true;
};