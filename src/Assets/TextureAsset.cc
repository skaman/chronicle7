#include "TextureAsset.h"

#include "Locator.h"
#include "Renderer/Renderer.h"

namespace chronicle {

CHR_CONCRETE(TextureAsset);

TextureAsset::TextureAsset(const std::string& filename)
{
    int texWidth;
    int texHeight;
    int texChannels;

    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    uint32_t imageSize = static_cast<uint32_t>(texWidth) * static_cast<uint32_t>(texHeight) * 4;

    if (!pixels) {
        throw std::runtime_error(fmt::format("Failed to load texture image {}", filename));
    }

    _image = Image::createTexture({ .generateMipmaps = true });
    _image->set(pixels, imageSize, texWidth, texHeight);

    stbi_image_free(pixels);
}

TextureAssetRef TextureAsset::load(const std::string& filename)
{
    return std::make_shared<ConcreteTextureAsset>(filename);
}

} // namespace chronicle