// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Assets/Material.h"
#include "Assets/Mesh.h"

namespace chronicle {

struct AssetResult {
    std::vector<MeshRef> meshes = {};
};

class AssetLoader {
public:
    [[nodiscard]] static AssetResult load(const std::string& filename);

private:
    static Format getAttributeFormat(const tinygltf::Accessor& accessor);
    static IndexType getIndexType(Format format);
    static AttributeType getAttributeType(const std::string_view& attributeName);
    static uint32_t getLocationFromAttributeType(AttributeType attributeType);
    static TextureRef getTexture(const tinygltf::Model& model, uint32_t textureIndex);
};

} // namespace chronicle