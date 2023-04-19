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
    /// @brief Add a uniform object to the descriptor set.
    /// @tparam Tx Type of the uniform buffer.
    /// @param id Id used to identify the descriptor set, used later for update his value.
    /// @param stage Shader stage where to attach the uniform buffer.
    template <class Tx> void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        static_cast<T*>(this)->addUniform<Tx>(id, stage);
    }

    /// @brief Add a texture sampler to the descriptor set.
    /// @param stage Shader stage where to attach the texture sampler.
    /// @param texture Texture to attach to the descriptor set.
    void addSampler(ShaderStage stage, const TextureRef texture) { static_cast<T*>(this)->addSampler(stage, texture); }

    /// @brief Set data to a uniform buffer.
    /// @tparam Tx Type of the uniform buffer.
    /// @param id Id used to identify the descriptor set.
    /// @param data Data to copy into the descriptor set.
    template <class Tx> void setUniform(entt::hashed_string::hash_type id, const Tx& data)
    {
        static_cast<T*>(this)->setUniform<Tx>(id, data);
    }

    /// @brief Build the descriptor set. This must be called after all the structure information are added to the
    ///        descriptor set, and before bind it to the command buffer.
    ///        This MUST be called only once! If you need to change the descriptor set structure, you need to create
    ///        a new one.
    void build() { static_cast<T*>(this)->build(); }

    /// @brief Factory for create a new descriptor set.
    /// @return The descriptor set.
    [[nodiscard]] static DescriptorSetRef create() { return T::create(); }

private:
    DescriptorSetI() = default;
    friend T;
};

} // namespace chronicle