// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to define a material.
/// @tparam T Type with implementation.
template <class T> class MaterialI {
public:
    /// @brief The base color of the material.
    /// @return The base color.
    [[nodiscard]] glm::vec4 baseColorFactor() const { return static_cast<const T*>(this)->baseColorFactor(); }

    /// @brief Set the base color of the material.
    /// @param baseColorFactor The base color.
    void setBaseColorFactor(glm::vec4 baseColorFactor) { static_cast<T*>(this)->setBaseColorFactor(baseColorFactor); }

    /// @brief The metalness of the material. Values range from 0.0 (non-metal) to 1.0 (metal).
    /// @return The metalness value.
    [[nodiscard]] float metallicFactor() const { return static_cast<const T*>(this)->metallicFactor(); }

    /// @brief Set the metalness of the material. Values range from 0.0 (non-metal) to 1.0 (metal).
    /// @param metallicFactor The metalness value.
    void setMetallicFactor(float metallicFactor) { static_cast<T*>(this)->setMetallicFactor(metallicFactor); }

    /// @brief The roughness of the material. Values range from 0.0 (smooth) to 1.0 (rough).
    /// @return The roughness value.
    [[nodiscard]] float roughnessFactor() const { return static_cast<const T*>(this)->roughnessFactor(); }

    /// @brief Set the roughness of the material. Values range from 0.0 (smooth) to 1.0 (rough).
    /// @param roughnessFactor The roughness value.
    void setRoughnessFactor(float roughnessFactor) { static_cast<T*>(this)->setRoughnessFactor(roughnessFactor); }

    /// @brief The factors for the emissive color of the material.
    /// @return Emissive factor value.
    [[nodiscard]] glm::vec3 emissiveFactor() const { return static_cast<const T*>(this)->emissiveFactor(); }

    /// @brief Set the factors for the emissive color of the material.
    /// @param emissiveFactor Emissive factor value.
    void setEmissiveFactor(glm::vec3 emissiveFactor) { static_cast<T*>(this)->setEmissiveFactor(emissiveFactor); }

    /// @brief The alpha rendering mode of the material.
    /// @return The alpha rendering mode value.
    [[nodiscard]] AlphaMode alphaMode() const { return static_cast<const T*>(this)->alphaMode(); }

    /// @brief Set the alpha rendering mode of the material.
    /// @param alphaMode The alpha rendering mode value.
    void setAlphaMode(AlphaMode alphaMode) { static_cast<T*>(this)->setAlphaMode(alphaMode); }

    /// @brief The alpha cutoff value of the material.
    /// @return The alpha cutoff value.
    [[nodiscard]] float alphaCutoff() const { return static_cast<const T*>(this)->alphaCutoff(); }

    /// @brief Set the alpha cutoff value of the material.
    /// @param alphaCutoff The alpha cutoff value.
    void setAlphaCutoff(float alphaCutoff) { static_cast<T*>(this)->setAlphaCutoff(alphaCutoff); }

    /// @brief Specifies whether the material is double sided.
    /// @return Double sided value.
    [[nodiscard]] bool doubleSided() const { return static_cast<const T*>(this)->doubleSided(); }

    /// @brief Set whether the material is double sided.
    /// @param doubleSided Double sided value.
    void setDoubleSided(bool doubleSided) { static_cast<T*>(this)->setDoubleSided(doubleSided); }

    /// @brief The base color texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& baseColorTexture() const { return static_cast<const T*>(this)->baseColorTexture(); }

    /// @brief Set the base color texture.
    /// @param baseColorTexture The texture.
    void setBaseColorTexture(const TextureRef& baseColorTexture)
    {
        static_cast<T*>(this)->setBaseColorTexture(baseColorTexture);
    }

    /// @brief The metallic-roughness texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& metallicRoughnessTexture() const
    {
        return static_cast<const T*>(this)->metallicRoughnessTexture();
    }

    /// @brief Set the metallic-roughness texture.
    /// @param metallicRoughnessTexture The texture.
    void setMetallicRoughnessTexture(const TextureRef& metallicRoughnessTexture)
    {
        static_cast<T*>(this)->setMetallicRoughnessTexture(metallicRoughnessTexture);
    }

    /// @brief The tangent space normal texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& normalTexture() const { return static_cast<const T*>(this)->normalTexture(); }

    /// @brief Set the tangent space normal texture.
    /// @param normalTexture The texture.
    void setNormalTexture(const TextureRef& normalTexture) { static_cast<T*>(this)->setNormalTexture(normalTexture); }

    /// @brief The occlusion texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& occlusionTexture() const { return static_cast<const T*>(this)->occlusionTexture(); }

    /// @brief Set the occlusion texture.
    /// @param occlusionTexture The texture.
    void setOcclusionTexture(const TextureRef& occlusionTexture)
    {
        static_cast<T*>(this)->setOcclusionTexture(occlusionTexture);
    }

    /// @brief The emissive texture.
    /// @return The texture.
    [[nodiscard]] const TextureRef& emissiveTexture() const { return static_cast<const T*>(this)->emissiveTexture(); }

    /// @brief Set the emissive texture.
    /// @param emissiveTexture The texture.
    void setEmissiveTexture(const TextureRef& emissiveTexture)
    {
        static_cast<T*>(this)->setEmissiveTexture(emissiveTexture);
    }

    /// @brief Build the material.
    void build() { return static_cast<T*>(this)->build(); }

    /// @brief Get the descriptor set for the material.
    /// @return The descriptor set.
    DescriptorSetRef descriptorSet() const { return static_cast<const T*>(this)->descriptorSet(); }

    /// @brief Factory for create a new material.
    /// @param debugName Debug name.
    /// @return The material.
    [[nodiscard]] static MaterialRef create(const char* debugName) { return T::create(debugName); }

private:
    MaterialI() = default;
    friend T;
};

} // namespace chronicle