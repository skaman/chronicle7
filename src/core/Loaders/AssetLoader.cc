// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "AssetLoader.h"

#include "PipelineLoader.h"
#include "ShaderLoader.h"

namespace chronicle {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord0;
    glm::vec2 texCoord1;
    glm::vec4 color;
};

AssetResult AssetLoader::load(const std::string& filename, const RenderPassRef& renderPass)
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

    // create materials
    for (const auto& gltfMaterial : model.materials) {
        auto material = createMaterial(model, gltfMaterial);
        materials.push_back(std::move(material));
    }

    // create meshes
    for (const auto& gltfMesh : model.meshes) {
        result.meshes.push_back(createMesh(model, gltfMesh, materials, defaultMaterial, renderPass));
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
    else if (attributeName == "TEXCOORD_1")
        return AttributeType::textcoord1;
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
    case chronicle::AttributeType::normal:
        return 1;
    case chronicle::AttributeType::textcoord0:
        return 2;
    case chronicle::AttributeType::textcoord1:
        return 3;
    case chronicle::AttributeType::color0:
        return 4;
    case chronicle::AttributeType::tangent:
        return 5;
    case chronicle::AttributeType::joints0:
        return 6;
    case chronicle::AttributeType::weights0:
        return 7;
    default:
        break;
    }

    return 0;
}

TextureRef AssetLoader::createTexture(const tinygltf::Model& gltfModel, uint32_t textureIndex)
{
    assert(gltfModel.textures.size() > textureIndex);

    const auto& gltfTexture = gltfModel.textures[textureIndex];
    auto imageIndex = gltfTexture.source;
    assert(gltfModel.images.size() > imageIndex);

    const auto& gltfImage = gltfModel.images[imageIndex];
    assert(gltfImage.image.size() > 0);
    assert(gltfImage.width > 0);
    assert(gltfImage.height > 0);

    // TODO: handle texture sampler
    auto texture = Texture::createSampled({ .generateMipmaps = true,
                                              .data = gltfImage.image,
                                              .width = static_cast<uint32_t>(gltfImage.width),
                                              .height = static_cast<uint32_t>(gltfImage.height) },
        fmt::format("{}", gltfModel.textures[textureIndex].name));
    return texture;
}

MaterialRef AssetLoader::createMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial)
{
    auto material = Material::create(gltfMaterial.name.c_str());

    // get the material parameters
    for (const auto& [name, parameter] : gltfMaterial.values) {
        if (name == "baseColorFactor") {
            const auto& colorFactor = parameter.ColorFactor();
            material->setBaseColorFactor(glm::vec4(colorFactor[0], colorFactor[1], colorFactor[2], colorFactor[3]));
        } else if (name == "metallicFactor") {
            material->setMetallicFactor(static_cast<float>(parameter.Factor()));
        } else if (name == "roughnessFactor") {
            material->setRoughnessFactor(static_cast<float>(parameter.Factor()));
        } else if (name == "baseColorTexture") {
            material->setBaseColorTexture(createTexture(gltfModel, parameter.TextureIndex()));
        } else if (name == "metallicRoughnessTexture") {
            material->setMetallicRoughnessTexture(createTexture(gltfModel, parameter.TextureIndex()));
        }
    }

    // get the additional parameters
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
            material->setNormalTexture(createTexture(gltfModel, parameter.TextureIndex()));
        } else if (name == "occlusionTexture") {
            material->setOcclusionTexture(createTexture(gltfModel, parameter.TextureIndex()));
        } else if (name == "emissiveTexture") {
            material->setEmissiveTexture(createTexture(gltfModel, parameter.TextureIndex()));
        }
    }

    material->build();
    return material;
}

MeshRef AssetLoader::createMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh,
    const std::vector<MaterialRef>& materials, const MaterialRef& defaultMaterial, const RenderPassRef& renderPass)
{
    std::vector<Submesh> submeshes = {};

    // every primitive is a submesh
    for (uint32_t primitiveIndex = 0; primitiveIndex < static_cast<uint32_t>(gltfMesh.primitives.size());
         primitiveIndex++) {
        const tinygltf::Primitive& gltfPrimitive = gltfMesh.primitives[primitiveIndex];

        auto submeshName = fmt::format("'{}' mesh, primitive #{}", gltfMesh.name, primitiveIndex);
        CHRLOG_DEBUG("{}", submeshName);

        Submesh submesh = {};

        const unsigned char* positionBuffer = nullptr;
        const unsigned char* normalBuffer = nullptr;
        const unsigned char* texCoord0Buffer = nullptr;
        const unsigned char* texCoord1Buffer = nullptr;
        const unsigned char* colorBuffer = nullptr;

        uint32_t positionStride = 0;
        uint32_t normalStride = 0;
        uint32_t texCoord0Stride = 0;
        uint32_t texCoord1Stride = 0;
        uint32_t colorStride = 0;

        // vertex buffers attributes
        for (const auto& [attributeName, accessorId] : gltfPrimitive.attributes) {
            const auto& accessor = gltfModel.accessors[accessorId];
            const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
            auto stride = accessor.ByteStride(bufferView);
            auto attributeType = getAttributeType(attributeName);

            assert(stride >= 0);

            if (attributeType == AttributeType::undefined)
                continue;

            if (attributeType == AttributeType::position) {
                submesh.verticesCount = static_cast<uint32_t>(accessor.count);
                submesh.boundingBox.min
                    = glm::vec3(accessor.minValues[0], accessor.minValues[1], accessor.minValues[2]);
                submesh.boundingBox.max
                    = glm::vec3(accessor.maxValues[0], accessor.maxValues[1], accessor.maxValues[2]);
            }

            const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];

            const auto* buffer = &gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset];

            switch (attributeType) {
            case chronicle::AttributeType::position:
                positionBuffer = buffer;
                positionStride = stride;
                break;
            case chronicle::AttributeType::normal:
                normalBuffer = buffer;
                normalStride = stride;
                break;
            case chronicle::AttributeType::textcoord0:
                texCoord0Buffer = buffer;
                texCoord0Stride = stride;
                break;
            case chronicle::AttributeType::textcoord1:
                texCoord1Buffer = buffer;
                texCoord1Stride = stride;
                break;
            case chronicle::AttributeType::color0:
                colorBuffer = buffer;
                colorStride = stride;
                break;
            default:
                break;
            }
        }

        assert(submesh.verticesCount > 0);
        assert(positionBuffer != nullptr);

        std::vector<Vertex> stagingVertexBuffer(submesh.verticesCount);

        for (auto i = 0; i < submesh.verticesCount; i++) {
            auto& vertex = stagingVertexBuffer[i];

            // TODO: make dynamic and try to convert the format (see getAttributeFormat)
            vertex.position = glm::make_vec3(std::bit_cast<const float*>(&positionBuffer[i * positionStride]));
            vertex.normal = normalBuffer ? glm::make_vec3(std::bit_cast<const float*>(&normalBuffer[i * normalStride]))
                                         : glm::vec3(0.0f);
            vertex.texCoord0 = texCoord0Buffer
                ? glm::make_vec2(std::bit_cast<const float*>(&texCoord0Buffer[i * texCoord0Stride]))
                : glm::vec2(0.0f);
            vertex.texCoord1 = texCoord1Buffer
                ? glm::make_vec2(std::bit_cast<const float*>(&texCoord1Buffer[i * texCoord1Stride]))
                : glm::vec2(0.0f);
            vertex.color = colorBuffer ? glm::make_vec4(std::bit_cast<const float*>(&colorBuffer[i * colorStride]))
                                       : glm::vec4(0.0f);
        }

        {
            auto vertexBuffer = VertexBuffer::create((uint8_t*)stagingVertexBuffer.data(),
                stagingVertexBuffer.size() * sizeof(Vertex),
                fmt::format("'{}' mesh, primitive #{}: vertex buffer", gltfMesh.name, primitiveIndex));
            submesh.vertexBuffers.push_back(vertexBuffer);
            submesh.vertexBufferIds.push_back(vertexBuffer->vertexBufferId());
            submesh.vertexBufferOffsets.push_back(0);

            VertexBufferInfo vertexBufferInfo = {};
            vertexBufferInfo.stride = sizeof(Vertex);
            vertexBufferInfo.attributeDescriptions
                = { { .format = Format::R32G32B32Sfloat,
                        .offset = offsetof(Vertex, position),
                        .location = getLocationFromAttributeType(AttributeType::position) },
                      { .format = Format::R32G32B32Sfloat,
                          .offset = offsetof(Vertex, normal),
                          .location = getLocationFromAttributeType(AttributeType::normal) },
                      { .format = Format::R32G32Sfloat,
                          .offset = offsetof(Vertex, texCoord0),
                          .location = getLocationFromAttributeType(AttributeType::textcoord0) },
                      { .format = Format::R32G32Sfloat,
                          .offset = offsetof(Vertex, texCoord1),
                          .location = getLocationFromAttributeType(AttributeType::textcoord1) },
                      { .format = Format::R32G32B32A32Sfloat,
                          .offset = offsetof(Vertex, color),
                          .location = getLocationFromAttributeType(AttributeType::color0) } };
            submesh.vertexBuffersInfo.push_back(vertexBufferInfo);
        }

        // create indices if availables
        if (gltfPrimitive.indices >= 0) {
            const auto& accessor = gltfModel.accessors[gltfPrimitive.indices];
            submesh.indicesCount = static_cast<uint32_t>(accessor.count);
            auto format = getAttributeFormat(accessor);
            submesh.indexType = getIndexType(format);

            if (submesh.indexType == IndexType::undefined) {
                CHRLOG_ERROR("Unsupported index type for mesh {}", gltfMesh.name);
                break;
            }

            const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
            auto stride = accessor.ByteStride(bufferView);
            const auto& buffer = gltfModel.buffers[bufferView.buffer];

            auto startByte = accessor.byteOffset + bufferView.byteOffset;
            auto endByte = accessor.count * stride;

            submesh.indexBuffer = IndexBuffer::create((uint8_t*)(buffer.data.data() + startByte), endByte,
                fmt::format("'{}' mesh, primitive #{}: index buffer", gltfMesh.name, primitiveIndex));
            submesh.indexBufferId = (vk::Buffer)submesh.indexBuffer->indexBufferId();
        }

        // set material
        if (gltfPrimitive.material < 0) {
            submesh.material = defaultMaterial;
        } else {
            assert(materials.size() > gltfPrimitive.material);
            submesh.material = materials[gltfPrimitive.material];
        }

        // create descriptor set layout
        DescriptorSetLayout descriptorLayout = {};
        descriptorLayout.setNumber = 1;
        descriptorLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 0,
            .descriptorType = DescriptorType::uniformBuffer,
            .descriptorCount = 1,
            .stageFlags = ShaderStage::fragment });

        // create pipeline
        ShaderCompilerOptions shaderCompilerOptions = {};
        shaderCompilerOptions.filename = ":/MaterialPbr.hlsl";
        if (submesh.material->haveBaseColorTexture()) {
            shaderCompilerOptions.macroDefinitions.emplace_back("HAS_BASE_COLOR_TEXTURE");
            descriptorLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 1,
                .descriptorType = DescriptorType::combinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::fragment });
        }
        if (submesh.material->haveMetallicRoughnessTexture()) {
            shaderCompilerOptions.macroDefinitions.emplace_back("HAS_METALLIC_ROUGHNESS_TEXTURE");
            descriptorLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 2,
                .descriptorType = DescriptorType::combinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::fragment });
        }
        if (submesh.material->haveNormalTexture()) {
            shaderCompilerOptions.macroDefinitions.emplace_back("HAS_NORMAL_TEXTURE");
            descriptorLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 3,
                .descriptorType = DescriptorType::combinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::fragment });
        }
        if (submesh.material->haveOcclusionTexture()) {
            shaderCompilerOptions.macroDefinitions.emplace_back("HAS_OCCLUSION_TEXTURE");
            descriptorLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 4,
                .descriptorType = DescriptorType::combinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::fragment });
        }
        if (submesh.material->haveEmissiveTexture()) {
            shaderCompilerOptions.macroDefinitions.emplace_back("HAS_EMISSIVE_TEXTURE");
            descriptorLayout.bindings.emplace_back(DescriptorSetLayoutBinding { .binding = 5,
                .descriptorType = DescriptorType::combinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::fragment });
        }

        PipelineInfo pipelineInfo = {};
        pipelineInfo.shader = ShaderLoader::load(shaderCompilerOptions);
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.vertexBuffers = submesh.vertexBuffersInfo;
        pipelineInfo.descriptorSetsLayout.push_back(Renderer::descriptorSetLayout());
        pipelineInfo.descriptorSetsLayout.push_back(descriptorLayout);
        submesh.pipeline = PipelineLoader::load(pipelineInfo, "test"); // TODO: handle debug name

        submeshes.push_back(std::move(submesh));
    }

    assert(submeshes.size() > 0);

    return Mesh::create(submeshes);
}

} // namespace chronicle