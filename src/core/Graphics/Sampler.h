// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

/// @brief Error exception for invalid sampler operations.
class SamplerError : public GraphicsError
{
  public:
    explicit SamplerError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Description for sampler creation.
struct SamplerDescriptor
{
    /// @brief Sampler name.
    std::string name{};

    /// @brief Specifies the address modes for the texture width.
    AddressMode addressModeU{AddressMode::eClampToEdge};

    /// @brief Specifies the address modes for the texture height.
    AddressMode addressModeV{AddressMode::eClampToEdge};

    /// @brief Specifies the address modes for the texture depth.
    AddressMode addressModeW{AddressMode::eClampToEdge};

    /// @brief Specifies the sampling behavior when the sample footprint is smaller than or equal to one texel.
    FilterMode magFilter{FilterMode::eNearest};

    /// @brief Specifies the sampling behavior when the sample footprint is larger than one texel.
    FilterMode minFilter{FilterMode::eNearest};

    /// @brief Specifies behavior for sampling between mipmap levels.
    MipmapFilterMode mipmapFilter{MipmapFilterMode::eNearest};

    /// @brief Specifies the minimum level of detail used internally when sampling a texture.
    float lodMinClamp{0.0f};

    /// @brief Specifies the maximum level of detail used internally when sampling a texture.
    float lodMaxClamp{32.0f};

    /// @brief When provided the sampler will be a comparison sampler with the specified CompareFunction.
    std::optional<CompareFunction> compare{};

    /// @brief Specifies the maximum anisotropy value clamp used by the sampler.
    uint32_t maxAnisotropy{1};
};

/// @brief A Sampler encodes transformations and filtering information that can be used in a shader to interpret texture
/// resource data.
class Sampler
{
  public:
    /// @brief Constructor.
    /// @param bufferDescriptor Buffer descriptor.
    explicit Sampler(const SamplerDescriptor &samplerDescriptor) : _samplerDescriptor(samplerDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~Sampler() = default;

    /// @brief Returns the descriptor used to create the sampler.
    /// @return Buffer descriptor.
    const SamplerDescriptor &descriptor() const
    {
        return _samplerDescriptor;
    }

  private:
    SamplerDescriptor _samplerDescriptor; ///< Sampler descriptor.
};

} // namespace chronicle::graphics
