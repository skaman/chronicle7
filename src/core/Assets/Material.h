// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Renderer.h"

namespace chronicle {

/// @brief Alpha mode.
enum class AlphaMode : uint32_t {
    opaque, ///< alpha value is ignored
    mask, ///< either full opaque of fully transparent
    blend ///< output is combined with the background
};

/// @brief Material UBO for descriptor set.
struct MaterialUBO {
    glm::vec4 baseColorFactor { 1.0f, 1.0f, 1.0f, 1.0f }; ///< The base color of the material.
    float metallicFactor { 1.0f }; ///< The metalness of the material.
    float roughnessFactor { 1.0f }; ///< The roughness of the material.
    glm::vec3 emissiveFactor { 0.0f, 0.0f, 0.0f }; ///< The factors for the emissive color of the material.
    AlphaMode alphaMode { AlphaMode::opaque }; ///< The alpha rendering mode of the material.
    float alphaCutoff { 0.5f }; ///< The alpha cutoff value of the material.
    bool doubleSided { false }; ///< Specifies whether the material is double sided.
};

class Material;
using MaterialRef = std::shared_ptr<Material>;

/// @brief Object used to define a material.
class Material : private NonCopyable<Material> {
protected:
    /// @brief Default constructor.
    /// @param debugName Debug name.
    explicit Material(const char* debugName);

public:
    /// @brief Destructor.
    ~Material() = default;

    /// @brief The base color of the material.
    /// @return The base color.
    [[nodiscard]] glm::vec4 baseColorFactor() const { return _ubo.baseColorFactor; }

    /// @brief Set the base color of the material.
    /// @param baseColorFactor The base color.
    void setBaseColorFactor(glm::vec4 baseColorFactor) { _ubo.baseColorFactor = baseColorFactor; }

    /// @brief The metalness of the material. Values range from 0.0 (non-metal) to 1.0 (metal).
    /// @return The metalness value.
    [[nodiscard]] float metallicFactor() const { return _ubo.metallicFactor; }

    /// @brief Set the metalness of the material. Values range from 0.0 (non-metal) to 1.0 (metal).
    /// @param metallicFactor The metalness value.
    void setMetallicFactor(float metallicFactor) { _ubo.metallicFactor = metallicFactor; }

    /// @brief The roughness of the material. Values range from 0.0 (smooth) to 1.0 (rough).
    /// @return The roughness value.
    [[nodiscard]] float roughnessFactor() const { return _ubo.roughnessFactor; }

    /// @brief Set the roughness of the material. Values range from 0.0 (smooth) to 1.0 (rough).
    /// @param roughnessFactor The roughness value.
    void setRoughnessFactor(float roughnessFactor) { _ubo.roughnessFactor = roughnessFactor; }

    /// @brief The factors for the emissive color of the material.
    /// @return Emissive factor value.
    [[nodiscard]] glm::vec3 emissiveFactor() const { return _ubo.emissiveFactor; }

    /// @brief Set the factors for the emissive color of the material.
    /// @param emissiveFactor Emissive factor value.
    void setEmissiveFactor(glm::vec3 emissiveFactor) { _ubo.emissiveFactor = emissiveFactor; }

    /// @brief The alpha rendering mode of the material.
    /// @return The alpha rendering mode value.
    [[nodiscard]] AlphaMode alphaMode() const { return _ubo.alphaMode; }

    /// @brief Set the alpha rendering mode of the material.
    /// @param alphaMode The alpha rendering mode value.
    void setAlphaMode(AlphaMode alphaMode) { _ubo.alphaMode = alphaMode; }

    /// @brief The alpha cutoff value of the material.
    /// @return The alpha cutoff value.
    [[nodiscard]] float alphaCutoff() const { return _ubo.alphaCutoff; }

    /// @brief Set the alpha cutoff value of the material.
    /// @param alphaCutoff The alpha cutoff value.
    void setAlphaCutoff(float alphaCutoff) { _ubo.alphaCutoff = alphaCutoff; }

    /// @brief Specifies whether the material is double sided.
    /// @return Double sided value.
    [[nodiscard]] bool doubleSided() const { return _ubo.doubleSided; }

    /// @brief Set whether the material is double sided.
    /// @param doubleSided Double sided value.
    void setDoubleSided(bool doubleSided) { _ubo.doubleSided = doubleSided; }

    /// @brief The base color texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& baseColorTexture() const { return _baseColorTexture; }

    /// @brief Set the base color texture.
    /// @param baseColorTexture The texture.
    void setBaseColorTexture(const TextureRef& baseColorTexture) { _baseColorTexture = baseColorTexture; }

    /// @brief Check if the base color texture is available.
    /// @return True if available.
    bool haveBaseColorTexture() { return _baseColorTexture != nullptr; }

    /// @brief The metallic-roughness texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& metallicRoughnessTexture() const { return _metallicRoughnessTexture; }

    /// @brief Set the metallic-roughness texture.
    /// @param metallicRoughnessTexture The texture.
    void setMetallicRoughnessTexture(const TextureRef& metallicRoughnessTexture)
    {
        _metallicRoughnessTexture = metallicRoughnessTexture;
    }

    /// @brief Check if the metallic-roughness texture is available.
    /// @return True if available.
    bool haveMetallicRoughnessTexture() { return _metallicRoughnessTexture != nullptr; }

    /// @brief The tangent space normal texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& normalTexture() const { return _normalTexture; }

    /// @brief Set the tangent space normal texture.
    /// @param normalTexture The texture.
    void setNormalTexture(const TextureRef& normalTexture) { _normalTexture = normalTexture; }

    /// @brief Check if the normal texture is available.
    /// @return True if available.
    bool haveNormalTexture() { return _normalTexture != nullptr; }

    /// @brief The occlusion texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& occlusionTexture() const { return _occlusionTexture; }

    /// @brief Set the occlusion texture.
    /// @param occlusionTexture The texture.
    void setOcclusionTexture(const TextureRef& occlusionTexture) { _occlusionTexture = occlusionTexture; }

    /// @brief Check if the occlusion texture is available.
    /// @return True if available.
    bool haveOcclusionTexture() { return _occlusionTexture != nullptr; }

    /// @brief The emissive texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& emissiveTexture() const { return _emissiveTexture; }

    /// @brief Set the emissive texture.
    /// @param emissiveTexture The texture.
    void setEmissiveTexture(const TextureRef& emissiveTexture) { _emissiveTexture = emissiveTexture; }

    /// @brief Check if the emissive texture is available.
    /// @return True if available.
    bool haveEmissiveTexture() { return _emissiveTexture != nullptr; }

    /// @brief Build the material.
    void build();

    /// @brief Get the descriptor set for the material.
    /// @return The descriptor set.
    DescriptorSetRef descriptorSet() const { return _descriptorSet; }

    /// @brief Factory for create a new material.
    /// @param debugName Debug name.
    /// @return The material.
    [[nodiscard]] static MaterialRef create(const char* debugName);

private:
    MaterialUBO _ubo {}; ///< Material uniform buffer object.
    TextureRef _baseColorTexture {}; ///< The base color texture.
    TextureRef _metallicRoughnessTexture {}; ///< The metallic-roughness texture.
    TextureRef _normalTexture {}; ///< The tangent space normal texture.
    TextureRef _occlusionTexture {}; ///< The occlusion texture.
    TextureRef _emissiveTexture {}; ///< The emissive texture.
    DescriptorSetRef _descriptorSet {}; ///< Descriptor set.
};

} // namespace chronicle
