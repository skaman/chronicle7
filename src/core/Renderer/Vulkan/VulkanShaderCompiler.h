// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/ShaderCompilerI.h"

#include <shaderc/shaderc.hpp>

namespace chronicle {

/// @brief Vulkan implementation for @ref ShaderCompilerI
class VulkanShaderCompiler : public ShaderCompilerI<VulkanShaderCompiler>, private NonCopyable<VulkanShaderCompiler> {
public:
    /// @brief @see ShaderCompilerI#compile
    [[nodiscard]] static ShaderRef compile(const ShaderCompilerOptions& options);

private:
    /// @brief Get shaderc shader kind from shader stage.
    /// @param stage Shader stage.
    /// @return Shader kind.
    [[nodiscard]] static shaderc_shader_kind getSpirvShader(ShaderStage stage);

    /// @brief Get entry point from code for a specific shader stage.
    /// @param sourceCode Source code.
    /// @param shaderStage Shader stage.
    /// @return Entry point.
    [[nodiscard]] static std::string getEntryPoint(const std::string& sourceCode, ShaderStage shaderStage);

    /// @brief Compile a shader.
    /// @param sourceCode Source code to compile.
    /// @param options Shader compiler options.
    /// @param shaderStage Shader stage.
    /// @param entryPoint Entry point.
    /// @param options Compiler options.
    /// @return Compiled code.
    [[nodiscard]] static std::vector<uint8_t> compile(const std::string_view& sourceCode,
        const ShaderCompilerOptions& options, ShaderStage shaderStage, const std::string& entryPoint);
};

} // namespace chronicle