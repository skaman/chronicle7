// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "RendererError.h"
#include "Storage/Storage.h"
#include "TextureInfo.h"

namespace chronicle {

/// @brief Object used to handle a texture.
/// @tparam T Type with implementation.
template <class T> class TextureI {
public:
    /// @brief Get the texture handle ID
    /// @return Texture ID
    [[nodiscard]] TextureId textureId() const { return CRTP_CONST_THIS->textureId(); }

    /// @brief Get the sampler handle ID
    /// @return Sampler ID
    [[nodiscard]] SamplerId samplerId() const { return CRTP_CONST_THIS->samplerId(); }

    /// @brief Factory for create a new sampled texture.
    /// @param textureInfo Informations used to create the texture.
    /// @param name Texture name.
    /// @return The texture.
    [[nodiscard]] static TextureRef createSampled(const SampledTextureInfo& textureInfo, const std::string& name)
    {
        return T::createSampled(textureInfo, name);
    }

    /// @brief Factory for create a new color texture.
    /// @param textureInfo Informations used to create the texture.
    /// @param name Texture name.
    /// @return The texture.
    [[nodiscard]] static TextureRef createColor(const ColorTextureInfo& textureInfo, const std::string& name)
    {
        return T::createColor(textureInfo, name);
    }

    /// @brief Factory for create a new depth texture.
    /// @param textureInfo Informations used to create the texture.
    /// @param name Texture name.
    /// @return The texture.
    [[nodiscard]] static TextureRef createDepth(const DepthTextureInfo& textureInfo, const std::string& name)
    {
        return T::createDepth(textureInfo, name);
    }

    // TODO: how the hell i should handle texture? color and depth maybe part of the framebuffer?
    [[nodiscard]] static TextureRef createFromFile(const std::string& filename)
    {
        CHRLOG_DEBUG("Loading texture: {}", filename);

        int texWidth;
        int texHeight;
        int texChannels;

        // TODO: need a lot of performance improvements

        auto data = Storage::readBytes(filename);

        stbi_uc* pixels = stbi_load_from_memory(
            data.data(), static_cast<int>(data.size()), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        uint32_t imageSize = static_cast<uint32_t>(texWidth) * static_cast<uint32_t>(texHeight) * texChannels;

        if (!pixels) {
            throw RendererError(fmt::format("Failed to load texture image {}", filename));
        }

        auto finalData = std::vector<uint8_t>(imageSize);
        std::memcpy(finalData.data(), pixels, imageSize);

        auto texture = T::createSampled({ .generateMipmaps = true,
                                            .data = finalData,
                                            .width = static_cast<uint32_t>(texWidth),
                                            .height = static_cast<uint32_t>(texHeight) },
            filename);

        stbi_image_free(pixels);
        return texture;
    }

private:
    TextureI() = default;
    friend T;
};

} // namespace chronicle