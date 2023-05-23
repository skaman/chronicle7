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

    /// @brief Detect the shader language from the filename extensions.
    /// @param filename Shader file name.
    /// @return Shader language.
    [[nodiscard]] static std::optional<shaderc_source_language> detectShaderLanguage(const std::string_view& filename);

    /// @brief Detect the pragma stage for a line.
    ///        If the line contain the #pragma stage marker, it return the specified stage.
    ///        Otherwise it return none.
    /// @param line Line to parse.
    /// @return Shader stage.
    [[nodiscard]] static ShaderStage detectPragmaStage(const std::string& line);

    /// @brief Cleanup the source code (already preprocessed) from code related to other stages.
    /// @param source Source to cleanup.
    /// @param shaderStage Stage to keep.
    /// @return Cleaned source code.
    [[nodiscard]] static std::string cleanSourceFromOtherStages(const std::string& source, ShaderStage shaderStage);

    /// @brief Compile a shader.
    /// @param sourceCode Source code to compile.
    /// @param shaderLanguage Shader language.
    /// @param options Shader compiler options.
    /// @param shaderStage Shader stage.
    /// @param entryPoint Entry point.
    /// @param options Compiler options.
    /// @return Compiled code.
    [[nodiscard]] static std::vector<uint8_t> compile(const std::string_view& sourceCode,
        shaderc_source_language shaderLanguage, const ShaderCompilerOptions& options, ShaderStage shaderStage,
        const std::string& entryPoint);
};

} // namespace chronicle