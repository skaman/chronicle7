// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AssetLoader.h"

#include "PipelineLoader.h"
#include "ShaderLoader.h"

namespace chronicle {

AssetResult AssetLoader::load(const std::string& filename)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    CHRLOG_DEBUG("Load mesh: {}", filename);

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename.c_str());
    // bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return {};
    }

    AssetResult result = {};
    std::vector<MaterialRef> materials = {};
    materials.reserve(model.materials.size());

    auto defaultMaterial = Material::create("Default material");

    for (const auto& gltfMaterial : model.materials) {
        auto material = Material::create(gltfMaterial.name.c_str());

        for (const auto& [name, parameter] : gltfMaterial.values) {
            if (name == "baseColorFactor") {
                const auto& colorFactor = parameter.ColorFactor();
                material->setBaseColorFactor(glm::vec4(colorFactor[0], colorFactor[1], colorFactor[2], colorFactor[3]));
            } else if (name == "metallicFactor") {
                material->setMetallicFactor(static_cast<float>(parameter.Factor()));
            } else if (name == "roughnessFactor") {
                material->setRoughnessFactor(static_cast<float>(parameter.Factor()));
            } else if (name == "baseColorTexture") {
                material->setBaseColorTexture(getTexture(model, parameter.TextureIndex()));
            } else if (name == "metallicRoughnessTexture") {
                material->setMetallicRoughnessTexture(getTexture(model, parameter.TextureIndex()));
            }
        }

        for (const auto& [name, parameter] : gltfMaterial.additionalValues) {
            if (name == "emissiveFactor") {
                const auto& emissiveFactor = parameter.number_array;
                material->setEmissiveFactor(glm::vec3(emissiveFactor[0], emissiveFactor[1], emissiveFactor[2]));
            } else if (name == "alphaMode") {
                if (parameter.string_value == "BLEND") {
                    material->setAlphaMode(AlphaMode::blend);
                } else if (parameter.string_value == "OPAQUE") {
                    material->setAlphaMode(AlphaMode::opaque);
                } else if (parameter.string_value == "MASK") {
                    material->setAlphaMode(AlphaMode::mask);
                }
            } else if (name == "alphaCutoff") {
                material->setAlphaCutoff(static_cast<float>(parameter.number_value));
            } else if (name == "doubleSided") {
                material->setDoubleSided(parameter.bool_value);
            } else if (name == "normalTexture") {
                material->setNormalTexture(getTexture(model, parameter.TextureIndex()));
            } else if (name == "occlusionTexture") {
                material->setOcclusionTexture(getTexture(model, parameter.TextureIndex()));
            } else if (name == "emissiveTexture") {
                material->setEmissiveTexture(getTexture(model, parameter.TextureIndex()));
            }
        }

        material->build();
        materials.push_back(std::move(material));
    }

    for (const auto& gltfMesh : model.meshes) {
        std::vector<Submesh> submeshes = {};

        for (uint32_t primitiveIndex = 0; primitiveIndex < static_cast<uint32_t>(gltfMesh.primitives.size());
             primitiveIndex++) {
            const tinygltf::Primitive& primitive = gltfMesh.primitives[primitiveIndex];

            auto submeshName = fmt::format("'{}' mesh, primitive #{}", gltfMesh.name, primitiveIndex);
            CHRLOG_DEBUG("{}", submeshName);

            Submesh submesh = {};

            std::map<uint32_t, VertexBufferRef> vertexBuffers = {};
            std::unordered_map<uint32_t, VertexBufferInfo> vertexBuffersInfo = {};

            for (const auto& [attributeName, accessorId] : primitive.attributes) {

                const auto& accessor = model.accessors[accessorId];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                auto stride = accessor.ByteStride(bufferView);
                auto attributeType = getAttributeType(attributeName);

                if (attributeType == AttributeType::undefined)
                    continue;

                if (attributeType == AttributeType::position)
                    submesh.verticesCount = static_cast<uint32_t>(accessor.count);

                if (!vertexBuffers.contains(accessor.bufferView)) {
#ifdef VULKAN_ENABLE_DEBUG_MARKER
                    auto tmpDebugName = fmt::format(
                        "'{}' mesh, primitive #{}: '{}' vertex buffer", gltfMesh.name, primitiveIndex, attributeName);
                    const char* debugName = tmpDebugName.c_str();
#else
                    const char* debugName = nullptr;
#endif // VULKAN_ENABLE_DEBUG_MARKER

                    const auto& buffer = model.buffers[bufferView.buffer];

                    auto vertexBuffer = VertexBuffer::create();
                    vertexBuffer->set(
                        (void*)(buffer.data.data() + bufferView.byteOffset), bufferView.byteLength, debugName);

                    vertexBuffers.try_emplace(accessor.bufferView, vertexBuffer);
                }

                if (!vertexBuffersInfo.contains(accessor.bufferView))
                    vertexBuffersInfo.try_emplace(accessor.bufferView);

                VertexBufferInfo& vertexBufferInfo = vertexBuffersInfo[accessor.bufferView];
                vertexBufferInfo.stride = static_cast<uint32_t>(stride);
                vertexBufferInfo.attributeDescriptions.emplace_back(
                    AttributeDescriptionInfo { .format = getAttributeFormat(accessor),
                        .offset = static_cast<uint32_t>(accessor.byteOffset),
                        .location = getLocationFromAttributeType(attributeType) });
            }

            std::vector<VertexBufferRef> tmpVertexBuffers = {};
            std::vector<uint32_t> tmpVertexBuffersOffsets = {};
            tmpVertexBuffers.reserve(vertexBuffers.size());
            tmpVertexBuffersOffsets.reserve(vertexBuffers.size());

            for (const auto& [id, buffer] : vertexBuffers) {
                tmpVertexBuffers.push_back(buffer);
                tmpVertexBuffersOffsets.push_back(0);
                submesh.vertexBuffersInfo.push_back(vertexBuffersInfo[id]);
            }
            submesh.vertexBuffers = VertexBuffers::create(tmpVertexBuffers, tmpVertexBuffersOffsets);

            if (primitive.indices >= 0) {
                const auto& accessor = model.accessors[primitive.indices];
                submesh.indicesCount = static_cast<uint32_t>(accessor.count);
                auto format = getAttributeFormat(accessor);
                submesh.indexType = getIndexType(format);

                if (submesh.indexType == IndexType::undefined) {
                    CHRLOG_ERROR("Unsupported index type for mesh {}", gltfMesh.name);
                    break;
                }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
                auto tmpDebugName
                    = fmt::format("'{}' mesh, primitive #{}: index buffer", gltfMesh.name, primitiveIndex);
                const char* debugName = tmpDebugName.c_str();
#else
                const char* debugName = nullptr;
#endif // VULKAN_ENABLE_DEBUG_MARKER

                const auto& bufferView = model.bufferViews[accessor.bufferView];
                auto stride = accessor.ByteStride(bufferView);
                const auto& buffer = model.buffers[bufferView.buffer];

                auto startByte = accessor.byteOffset + bufferView.byteOffset;
                auto endByte = accessor.count * stride;

                submesh.indexBuffer = IndexBuffer::create();
                submesh.indexBuffer->set((void*)(buffer.data.data() + startByte), endByte, debugName);
            }

            if (primitive.material < 0) {
                submesh.material = defaultMaterial;
            } else {
                assert(materials.size() > primitive.material);
                submesh.material = materials[primitive.material];
            }

            // create pipeline
            ShaderCompilerOptions shaderCompilerOptions = {};
            shaderCompilerOptions.filename = ":/MaterialPbr.hlsl";
            if (submesh.material->haveBaseColorTexture()) {
                shaderCompilerOptions.macroDefinitions.emplace_back("HAS_BASE_COLOR_TEXTURE");
            }
            if (submesh.material->haveMetallicRoughnessTexture()) {
                shaderCompilerOptions.macroDefinitions.emplace_back("HAS_METALLIC_ROUGHNESS_TEXTURE");
            }
            if (submesh.material->haveNormalTexture()) {
                shaderCompilerOptions.macroDefinitions.emplace_back("HAS_NORMAL_TEXTURE");
            }
            if (submesh.material->haveOcclusionTexture()) {
                shaderCompilerOptions.macroDefinitions.emplace_back("HAS_OCCLUSION_TEXTURE");
            }
            if (submesh.material->haveEmissiveTexture()) {
                shaderCompilerOptions.macroDefinitions.emplace_back("HAS_EMISSIVE_TEXTURE");
            }

            PipelineInfo pipelineInfo = {};
            pipelineInfo.shader = ShaderLoader::load(shaderCompilerOptions);
            pipelineInfo.vertexBuffers = submesh.vertexBuffersInfo;
            submesh.pipeline = PipelineLoader::load(pipelineInfo, "test"); // TODO: handle debug name

            submeshes.push_back(std::move(submesh));
        }

        result.meshes.push_back(Mesh::create(submeshes));
    }

    return result;
}

Format AssetLoader::getAttributeFormat(const tinygltf::Accessor& accessor)
{
    switch (accessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R8Sint }, { TINYGLTF_TYPE_VEC2, Format::R8G8Sint },
                  { TINYGLTF_TYPE_VEC3, Format::R8G8B8Sint }, { TINYGLTF_TYPE_VEC4, Format::R8G8B8A8Sint } };

        return mappedFormat.at(accessor.type);
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R8Uint }, { TINYGLTF_TYPE_VEC2, Format::R8G8Uint },
                  { TINYGLTF_TYPE_VEC3, Format::R8G8B8Uint }, { TINYGLTF_TYPE_VEC4, Format::R8G8B8A8Uint } };

        static const std::map<int, Format> mappedFormatNormalize
            = { { TINYGLTF_TYPE_SCALAR, Format::R8Unorm }, { TINYGLTF_TYPE_VEC2, Format::R8G8Unorm },
                  { TINYGLTF_TYPE_VEC3, Format::R8G8B8Unorm }, { TINYGLTF_TYPE_VEC4, Format::R8G8B8A8Unorm } };

        return accessor.normalized ? mappedFormatNormalize.at(accessor.type) : mappedFormat.at(accessor.type);
    }
    case TINYGLTF_COMPONENT_TYPE_SHORT: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R16Sint }, { TINYGLTF_TYPE_VEC2, Format::R16G16Sint },
                  { TINYGLTF_TYPE_VEC3, Format::R16G16B16Sint }, { TINYGLTF_TYPE_VEC4, Format::R16G16B16A16Sint } };

        return mappedFormat.at(accessor.type);
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R16Uint }, { TINYGLTF_TYPE_VEC2, Format::R16G16Uint },
                  { TINYGLTF_TYPE_VEC3, Format::R16G16B16Uint }, { TINYGLTF_TYPE_VEC4, Format::R16G16B16A16Uint } };

        static const std::map<int, Format> mappedFormatNormalize
            = { { TINYGLTF_TYPE_SCALAR, Format::R16Unorm }, { TINYGLTF_TYPE_VEC2, Format::R16G16Unorm },
                  { TINYGLTF_TYPE_VEC3, Format::R16G16B16Unorm }, { TINYGLTF_TYPE_VEC4, Format::R16G16B16A16Unorm } };

        return accessor.normalized ? mappedFormatNormalize.at(accessor.type) : mappedFormat.at(accessor.type);
    }
    case TINYGLTF_COMPONENT_TYPE_INT: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R32Sint }, { TINYGLTF_TYPE_VEC2, Format::R32G32Sint },
                  { TINYGLTF_TYPE_VEC3, Format::R32G32B32Sint }, { TINYGLTF_TYPE_VEC4, Format::R32G32B32A32Sint } };

        return mappedFormat.at(accessor.type);
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R32Uint }, { TINYGLTF_TYPE_VEC2, Format::R32G32Uint },
                  { TINYGLTF_TYPE_VEC3, Format::R32G32B32Uint }, { TINYGLTF_TYPE_VEC4, Format::R32G32B32A32Uint } };

        return mappedFormat.at(accessor.type);
    }
    case TINYGLTF_COMPONENT_TYPE_FLOAT: {
        static const std::map<int, Format> mappedFormat
            = { { TINYGLTF_TYPE_SCALAR, Format::R32Sfloat }, { TINYGLTF_TYPE_VEC2, Format::R32G32Sfloat },
                  { TINYGLTF_TYPE_VEC3, Format::R32G32B32Sfloat }, { TINYGLTF_TYPE_VEC4, Format::R32G32B32A32Sfloat } };

        return mappedFormat.at(accessor.type);
    }
    default:
        return Format::undefined;
    }
}

IndexType AssetLoader::getIndexType(Format format)
{
    switch (format) {
    case chronicle::Format::R16Uint:
        return IndexType::uint16;
    case chronicle::Format::R32Uint:
        return IndexType::uint32;
    default:
        return IndexType::undefined;
    }
}

AttributeType AssetLoader::getAttributeType(const std::string_view& attributeName)
{
    if (attributeName == "POSITION")
        return AttributeType::position;
    else if (attributeName == "NORMAL")
        return AttributeType::normal;
    else if (attributeName == "TEXCOORD_0")
        return AttributeType::textcoord0;
    else if (attributeName == "COLOR_0")
        return AttributeType::color0;
    else if (attributeName == "TANGENT")
        return AttributeType::tangent;
    else if (attributeName == "JOINTS_0")
        return AttributeType::joints0;
    else if (attributeName == "WEIGHTS_0")
        return AttributeType::weights0;

    return AttributeType::undefined;
}

uint32_t AssetLoader::getLocationFromAttributeType(AttributeType attributeType)
{
    switch (attributeType) {
    case chronicle::AttributeType::position:
        return 0;
    case chronicle::AttributeType::color0:
        return 1;
    case chronicle::AttributeType::textcoord0:
        return 2;
    case chronicle::AttributeType::normal:
        return 3;
    case chronicle::AttributeType::tangent:
        return 4;
    case chronicle::AttributeType::joints0:
        return 5;
    case chronicle::AttributeType::weights0:
        return 6;
    default:
        break;
    }

    return 0;
}

TextureRef AssetLoader::getTexture(const tinygltf::Model& model, uint32_t textureIndex)
{
    assert(model.textures.size() > textureIndex);

    const auto& glfwTexture = model.textures[textureIndex];
    auto imageIndex = glfwTexture.source;
    assert(model.images.size() > imageIndex);

    const auto& glfwImage = model.images[imageIndex];
    assert(glfwImage.image.size() > 0);
    assert(glfwImage.width > 0);
    assert(glfwImage.height > 0);

    // TODO: handle texture sampler
    auto texture = Texture::create({});
    texture->set((void*)glfwImage.image.data(), glfwImage.image.size(), glfwImage.width, glfwImage.height);
    return texture;
}

} // namespace chronicle