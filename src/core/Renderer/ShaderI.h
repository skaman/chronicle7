// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "DescriptorSetLayout.h"

namespace chronicle {

/// @brief Object used to handle a shader.
/// @tparam T Type with implementation.
template <class T> class ShaderI {
public:
    /// @brief Get the descriptor set layouts used by the shader.
    /// @return Descriptor set layouts.
    [[nodiscard]] std::vector<DescriptorSetLayout> descriptorSetLayouts() const
    {
        return CRTP_CONST_THIS->descriptorSetLayouts();
    };

    /// @brief Get the available shader stages.
    /// @return Shader stages.
    [[nodiscard]] std::vector<ShaderStage> stages() const { return CRTP_CONST_THIS->stages(); };

    /// @brief Get the hash of the configuration used to create the shader.
    /// @return Shaders hash.
    [[nodiscard]] size_t hash() const { return CRTP_CONST_THIS->hash(); };

    /// @brief Get the entry point for a specific shader stage.
    /// @param stage Shader stage.
    /// @return Entry point.
    [[nodiscard]] const std::string& entryPoint(ShaderStage stage) const
    {
        return CRTP_CONST_THIS->entryPoint(stage);
    }

private:
    ShaderI() = default;
    friend T;
};

} // namespace chronicle