// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to compile shaders.
/// @tparam T Type with implementation.
template <class T> class ShaderCompilerI {
public:
    /// @brief Compile a shader source with SPIR-V.
    /// @param filename Shader file name.
    /// @param options Shader compile options.
    /// @return Shader.
    [[nodiscard]] static ShaderRef compile(const std::string& filename) { return T::compile(filename); }

private:
    ShaderCompilerI() = default;
    friend T;
};

} // namespace chronicle