// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to manage a descriptor set.
/// @tparam T Type with implementation.
template <class T> class DescriptorSetI {
public:
    /// @brief Get the name for the descriptor set.
    /// @return Descriptor set name.
    [[nodiscard]] std::string name() const { CRTP_CONST_THIS->name(); }

    /// @brief Get the descriptor set handle ID
    /// @return Descriptor set ID
    [[nodiscard]] DescriptorSetId descriptorSetId() const { return CRTP_CONST_THIS->descriptorSetId(); }

    /// @brief Add a uniform object to the descriptor set.
    /// @tparam Tx Type of the uniform buffer.
    /// @param id Id used to identify the descriptor set, used later for update his value.
    /// @param stage Shader stage where to attach the uniform buffer.
    template <class Tx> void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        CRTP_THIS->addUniform<Tx>(id, stage);
    }

    /// @brief Add a texture sampler to the descriptor set.
    /// @param stage Shader stage where to attach the texture sampler.
    /// @param texture Texture to attach to the descriptor set.
    void addSampler(ShaderStage stage, const TextureRef texture) { CRTP_THIS->addSampler(stage, texture); }

    /// @brief Set data to a uniform buffer.
    /// @tparam Tx Type of the uniform buffer.
    /// @param id Id used to identify the descriptor set.
    /// @param data Data to copy into the descriptor set.
    template <class Tx> void setUniform(entt::hashed_string::hash_type id, const Tx& data)
    {
        CRTP_THIS->setUniform<Tx>(id, data);
    }

    /// @brief Build the descriptor set. This must be called after all the structure information are added to the
    ///        descriptor set, and before bind it to the command buffer.
    ///        This MUST be called only once! If you need to change the descriptor set structure, you need to create
    ///        a new one.
    void build() { CRTP_THIS->build(); }

    /// @brief Factory for create a new descriptor set.
    /// @param name Name.
    /// @return The descriptor set.
    [[nodiscard]] static DescriptorSetRef create(const std::string& name) { return T::create(name); }

private:
    DescriptorSetI() = default;
    friend T;
};

} // namespace chronicle