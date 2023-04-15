// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common.h"

namespace chronicle {

class TextureAsset;
using TextureAssetRef = std::shared_ptr<TextureAsset>;

class TextureAsset {
protected:
    explicit TextureAsset(const std::string& filename);

public:
    ~TextureAsset() = default;

    [[nodiscard]] static TextureAssetRef load(const std::string& filename);

    [[nodiscard]] const ImageRef& image() const { return _image; }

private:
    ImageRef _image;
};

} // namespace chronicle