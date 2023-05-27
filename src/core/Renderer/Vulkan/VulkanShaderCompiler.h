// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common/Common.h"
#include "Renderer/Data/DescriptorSetLayout.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace chronicle {

struct ShaderCompilerOptions {
    std::string filename {};
    std::vector<std::string> macroDefinitions {};
};

struct ShaderCompilerModule {
    std::vector<uint32_t> spirvBinary {};
    std::string entryPoint {};
};

constexpr int MaxDescriptorSetsCount { 4 };

struct ShaderCompilerResult {
    std::unordered_map<ShaderStage, ShaderCompilerModule> modules {};
    std::array<DescriptorSetLayout, MaxDescriptorSetsCount> descriptorSetsLayout;
};

/// @brief Vulkan implementation for @ref BaseShaderCompiler
class VulkanShaderCompiler : private NonCopyable<VulkanShaderCompiler> {
public:
    /// @brief Compile the shader with the given options.
    /// @param options Shader compile options.
    /// @return Data related to compiled shader, like spirv binary, entrypoints and descriptor sets.
    [[nodiscard]] static ShaderCompilerResult compile(const ShaderCompilerOptions& options);

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

    /// @brief Compile a shader module.
    /// @param sourceCode Source code to compile.
    /// @param shaderLanguage Shader language.
    /// @param options Shader compiler options.
    /// @param shaderStage Shader stage.
    /// @param descriptorSetsLayout Array containing the descriptor sets layouts.
    /// @return Compiled module.
    [[nodiscard]] static ShaderCompilerModule compileModule(const std::string_view& sourceCode,
        shaderc_source_language shaderLanguage, const ShaderCompilerOptions& options, ShaderStage shaderStage,
        std::array<DescriptorSetLayout, MaxDescriptorSetsCount>& descriptorSetsLayout);

    /// @brief Parse resources from SPIR-V compiler.
    /// @param compiler SPIR-V compiler.
    /// @param resource SPIR-V resource.
    /// @param filename Shader filename.
    /// @param shaderStage Shader stage.
    /// @param descriptorType Descriptor type.
    /// @return Descriptper set layout binding informations.
    [[nodiscard]] static DescriptorSetLayoutBinding parseResource(const spirv_cross::Compiler& compiler,
        const spirv_cross::Resource& resource, const std::string& filename, ShaderStage shaderStage,
        DescriptorType descriptorType);

    /// @brief Check if a resources is in use in the code.
    /// @param compiler SPIR-V compiler.
    /// @param resource SPIR-V resource.
    /// @return True if it's in use.
    [[nodiscard]] static bool isResourceInUse(
        const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource);

    /// @brief Add a binding to the descriptor set. It do the check for merge and compatibility.
    /// @param descriptorSetsLayout Descriptor sets layout to update.
    /// @param binding Binding to add.
    static void addBindingToDescriptorSet(std::array<DescriptorSetLayout, MaxDescriptorSetsCount>& descriptorSetsLayout,
        const DescriptorSetLayoutBinding& binding);

    /// @brief Get the type from the SPIR-V type.
    /// @param filename Shader filename.
    /// @param type SPIR-V type.
    /// @return Parsed type.
    [[nodiscard]] static Type typeFromSpirType(const std::string& filename, const spirv_cross::SPIRType& type);
};

} // namespace chronicle