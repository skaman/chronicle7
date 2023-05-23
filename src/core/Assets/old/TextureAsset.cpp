// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TextureAsset.h"

#include "Renderer/Renderer.h"

namespace chronicle {

CHR_CONCRETE(TextureAsset);

TextureAsset::TextureAsset(const std::string& filename)
{
    CHRZONE_ASSETS;

    assert(!filename.empty());

    CHRLOG_DEBUG("Loading texture: {}", filename);

    int texWidth;
    int texHeight;
    int texChannels;

    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    uint32_t imageSize = static_cast<uint32_t>(texWidth) * static_cast<uint32_t>(texHeight) * 4;

    if (!pixels) {
        throw std::runtime_error(fmt::format("Failed to load texture image {}", filename));
    }

    //_texture = Texture::create({ .generateMipmaps = true });
    //_texture->set(pixels, imageSize, texWidth, texHeight);

    stbi_image_free(pixels);
}

TextureAssetRef TextureAsset::load(const std::string& filename)
{
    return std::make_shared<ConcreteTextureAsset>(filename);
}

} // namespace chronicle