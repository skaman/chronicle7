// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

struct ShaderCompilerOptions {
    std::string filename = {};
    std::vector<std::string> macroDefinitions = {};
};

/// @brief Object used to compile shaders.
/// @tparam T Type with implementation.
template <class T> class ShaderCompilerI {
public:
    /// @brief Compile a shader source with SPIR-V.
    /// @param options Shader compiler options.
    /// @return Shader.
    [[nodiscard]] static ShaderRef compile(const ShaderCompilerOptions& options) { return T::compile(options); }

private:
    ShaderCompilerI() = default;
    friend T;
};

} // namespace chronicle

template <> struct std::hash<chronicle::ShaderCompilerOptions> {
    std::size_t operator()(const chronicle::ShaderCompilerOptions& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.filename);
        for (const auto& macroDefinition : data.macroDefinitions) {
            std::hash_combine(h, macroDefinition);
        }
        return h;
    }
};