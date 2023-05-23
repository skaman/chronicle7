// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseShader.h"

#pragma warning(push)
#pragma warning(disable : 26439)
#include <spirv-reflect/spirv_reflect.h>
#pragma warning(pop)

namespace chronicle {

/// @brief Vulkan implementation for @ref BaseShader
class VulkanShader : public BaseShader<VulkanShader>, private NonCopyable<VulkanShader> {
protected:
    /// @brief Default constructor.
    /// @param codes Shaders data compiled with SPIR-V.
    /// @param entryPoints Shaders entry points.
    /// @param hash Hash of the configuration used to create the shader.
    explicit VulkanShader(const std::unordered_map<ShaderStage, std::vector<uint8_t>>& codes,
        const std::unordered_map<ShaderStage, std::string>& entryPoints, size_t hash);

public:
    /// @brief Destructor.
    ~VulkanShader();

    /// @brief @see BaseShader#descriptorSetLayouts
    [[nodiscard]] std::vector<DescriptorSetLayout> descriptorSetLayouts() const { return _descriptorSetsLayout; }

    /// @brief @see BaseShader#stages
    [[nodiscard]] std::vector<ShaderStage> stages() const { return _stages; };

    /// @brief @see BaseShader#hash
    [[nodiscard]] size_t hash() const { return _hash; };

    /// @brief @see BaseShader#entryPoint
    [[nodiscard]] const std::string& entryPoint(ShaderStage stage) const { return _entryPoints.at(stage); }

    /// @brief Get the vulkan handle for the shader module.
    /// @param stage Shader stage.
    /// @return Vulkan handle.
    [[nodiscard]] const vk::ShaderModule& shaderModule(ShaderStage stage) const { return _shaderModules.at(stage); }

    /// @brief Check if a vulan handle exists for a specific stage.
    /// @param stage Shader stage.
    /// @return True if the handle exists.
    [[nodiscard]] bool shaderModuleExists(ShaderStage stage) const { return _shaderModules.contains(stage); }

    /// @brief Factory for create a new shader from SPIR-V code.
    /// @param codes Shaders data compiled with SPIR-V.
    /// @param entryPoints Shaders entry points.
    /// @param hash Hash of the configuration used to create the shader.
    /// @return The shader.
    [[nodiscard]] static ShaderRef create(const std::unordered_map<ShaderStage, std::vector<uint8_t>>& codes,
        const std::unordered_map<ShaderStage, std::string>& entryPoints, size_t hash);

private:
    std::unordered_map<ShaderStage, vk::ShaderModule> _shaderModules {}; ///< Shader modules mapped for stages.
    std::unordered_map<ShaderStage, std::string> _entryPoints {}; ///< Shader entry points.
    std::vector<DescriptorSetLayout> _descriptorSetsLayout {}; ///< Descriptor sets layout.
    std::vector<ShaderStage> _stages {}; ///< Shader stages.
    size_t _hash {}; ///< Hash of the configuration used to create the shader.

    /// @brief Get the descriptor sets layout data from SPIR-V code of a single shader.
    /// @param code Shaders SPIR-V code.
    /// @return Descriptor sets layout data.
    [[nodiscard]] static std::vector<DescriptorSetLayout> getDescriptorSetsLayout(const std::vector<uint8_t>& code);

    /// @brief Get the descriptor type from a SPIR-V reflect type.
    /// @param spvDescriptorType SPIR-V reflect type.
    /// @return Descriptor type.
    [[nodiscard]] static DescriptorType getDescriptorType(SpvReflectDescriptorType spvDescriptorType);

    /// @brief Get the shader stage from a SPIR-V reflect type.
    /// @param spvShaderStage SPIR-V reflect type.
    /// @return Shader stage.
    [[nodiscard]] static ShaderStage getShaderStage(SpvReflectShaderStageFlagBits spvShaderStage);
};

} // namespace chronicle