// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/MaterialI.h"

namespace chronicle {

/// @brief Material UBO for descriptor set.
struct MaterialUBO {
    glm::vec4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f }; ///< The base color of the material.
    float metallicFactor = 1.0f; ///< The metalness of the material.
    float roughnessFactor = 1.0f; ///< The roughness of the material.
    glm::vec3 emissiveFactor = { 0.0f, 0.0f, 0.0f }; ///< The factors for the emissive color of the material.
    AlphaMode alphaMode = AlphaMode::opaque; ///< The alpha rendering mode of the material.
    float alphaCutoff = 0.5f; ///< The alpha cutoff value of the material.
    bool doubleSided = false; ///< Specifies whether the material is double sided.
};

/// @brief Vulkan implementation for @ref MaterialI
class VulkanMaterial : public MaterialI<VulkanMaterial>, private NonCopyable<VulkanMaterial> {
protected:
    /// @brief Default constructor.
    /// @param debugName Debug name.
    explicit VulkanMaterial(const char* debugName);

public:
    /// @brief Destructor.
    ~VulkanMaterial() = default;

    /// @brief @see MaterialI#baseColorFactor
    [[nodiscard]] glm::vec4 baseColorFactor() const { return _ubo.baseColorFactor; }

    /// @brief @see MaterialI#setBaseColorFactor
    void setBaseColorFactor(glm::vec4 baseColorFactor) { _ubo.baseColorFactor = baseColorFactor; }

    /// @brief @see MaterialI#metallicFactor
    [[nodiscard]] float metallicFactor() const { return _ubo.metallicFactor; }

    /// @brief @see MaterialI#setMetallicFactor
    void setMetallicFactor(float metallicFactor) { _ubo.metallicFactor = metallicFactor; }

    /// @brief @see MaterialI#roughnessFactor
    [[nodiscard]] float roughnessFactor() const { return _ubo.roughnessFactor; }

    /// @brief @see MaterialI#setRoughnessFactor
    void setRoughnessFactor(float roughnessFactor) { _ubo.roughnessFactor = roughnessFactor; }

    /// @brief @see MaterialI#emissiveFactor
    [[nodiscard]] glm::vec3 emissiveFactor() const { return _ubo.emissiveFactor; }

    /// @brief @see MaterialI#setEmissiveFactor
    void setEmissiveFactor(glm::vec3 emissiveFactor) { _ubo.emissiveFactor = emissiveFactor; }

    /// @brief @see MaterialI#alphaMode
    [[nodiscard]] AlphaMode alphaMode() const { return _ubo.alphaMode; }

    /// @brief @see MaterialI#setAlphaMode
    void setAlphaMode(AlphaMode alphaMode) { _ubo.alphaMode = alphaMode; }

    /// @brief @see MaterialI#alphaCutoff
    [[nodiscard]] float alphaCutoff() const { return _ubo.alphaCutoff; }

    /// @brief @see MaterialI#setAlphaCutoff
    void setAlphaCutoff(float alphaCutoff) { _ubo.alphaCutoff = alphaCutoff; }

    /// @brief @see MaterialI#doubleSided
    [[nodiscard]] bool doubleSided() const { return _ubo.doubleSided; }

    /// @brief @see MaterialI#setDoubleSided
    void setDoubleSided(bool doubleSided) { _ubo.doubleSided = doubleSided; }

    /// @brief @see MaterialI#baseColorTexture
    [[nodiscard]] const TextureRef& baseColorTexture() const { return _baseColorTexture; }

    /// @brief @see MaterialI#setBaseColorTexture
    void setBaseColorTexture(const TextureRef& baseColorTexture) { _baseColorTexture = baseColorTexture; }

    /// @brief @see MaterialI#metallicRoughnessTexture
    [[nodiscard]] const TextureRef& metallicRoughnessTexture() const { return _metallicRoughnessTexture; }

    /// @brief @see MaterialI#setMetallicRoughnessTexture
    void setMetallicRoughnessTexture(const TextureRef& metallicRoughnessTexture)
    {
        _metallicRoughnessTexture = metallicRoughnessTexture;
    }

    /// @brief @see MaterialI#normalTexture
    [[nodiscard]] const TextureRef& normalTexture() const { return _normalTexture; }

    /// @brief @see MaterialI#setNormalTexture
    void setNormalTexture(const TextureRef& normalTexture) { _normalTexture = normalTexture; }

    /// @brief @see MaterialI#occlusionTexture
    [[nodiscard]] const TextureRef& occlusionTexture() const { return _occlusionTexture; }

    /// @brief @see MaterialI#setOcclusionTexture
    void setOcclusionTexture(const TextureRef& occlusionTexture) { _occlusionTexture = occlusionTexture; }

    /// @brief @see MaterialI#emissiveTexture
    [[nodiscard]] const TextureRef& emissiveTexture() const { return _emissiveTexture; }

    /// @brief @see MaterialI#setEmissiveTexture
    void setEmissiveTexture(const TextureRef& emissiveTexture) { _emissiveTexture = emissiveTexture; }

    /// @brief @see MaterialI#build
    void build();

    /// @brief @see MaterialI#descriptorSet
    DescriptorSetRef descriptorSet() const { return _descriptorSet; }

    /// @brief @see MaterialI#create
    [[nodiscard]] static MaterialRef create(const char* debugName);

private:
    MaterialUBO _ubo = {}; ///< Material uniform buffer object.
    TextureRef _baseColorTexture = {}; ///< The base color texture.
    TextureRef _metallicRoughnessTexture = {}; ///< The metallic-roughness texture.
    TextureRef _normalTexture = {}; ///< The tangent space normal texture.
    TextureRef _occlusionTexture = {}; ///< The occlusion texture.
    TextureRef _emissiveTexture = {}; ///< The emissive texture.
    DescriptorSetRef _descriptorSet = {}; ///< Descriptor set.
};

} // namespace chronicle
