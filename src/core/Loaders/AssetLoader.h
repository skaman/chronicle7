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

    static TextureRef createTexture(const tinygltf::Model& gltfModel, uint32_t textureIndex);
    static MaterialRef createMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial);

    static MeshRef createMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh,
        const std::vector<MaterialRef>& materials, const MaterialRef& defaultMaterial);
};

} // namespace chronicle