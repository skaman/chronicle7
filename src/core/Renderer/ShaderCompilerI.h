// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Compiler optimization level for shader compiler.
enum class ShaderOptimizationLevel {
    none, ///< No optimization.
    size, ///< Optimization for better size.
    performance ///< Optimization for better performance.
};

/// @brief Source language for shader files.
enum class ShaderSourceLanguage {
    glsl, ///< GLSL shader language.
    hlsl ///< HLSL shader language.
};

/// @brief Options for shader compiler.
struct ShaderCompilerOptions {
    /// @brief Compiler optimization level.
    ShaderOptimizationLevel optimization = ShaderOptimizationLevel::performance;

    /// @brief Shader file source language.
    ShaderSourceLanguage language = ShaderSourceLanguage::hlsl;

    /// @brief Treats all compiler warnings as errors.
    bool warningAsErrors = false;
};

/// @brief Object used to compile shaders.
/// @tparam T Type with implementation.
template <class T> class ShaderCompilerI {
public:
    /// @brief Compile a shader source with SPIR-V.
    /// @param filename Shader file name.
    /// @param options Shader compile options.
    /// @return Shader.
    [[nodiscard]] static ShaderRef compile(const std::string& filename, ShaderCompilerOptions options = {})
    {
        return T::compile(filename, options);
    }

private:
    ShaderCompilerI() = default;
    friend T;
};

} // namespace chronicle