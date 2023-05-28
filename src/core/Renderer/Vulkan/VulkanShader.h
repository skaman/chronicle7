// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseShader.h"

namespace chronicle::internal::vulkan {

/// @brief Vulkan implementation for @ref BaseShader
class VulkanShader : public BaseShader<VulkanShader>, private NonCopyable<VulkanShader> {
protected:
    /// @brief Default constructor.
    explicit VulkanShader(const ShaderInfo& shaderInfo);

public:
    /// @brief Destructor.
    ~VulkanShader() override;

    /// @brief @see BaseShader#descriptorSetLayouts
    [[nodiscard]] std::vector<DescriptorSetLayout> descriptorSetLayouts() const { return _descriptorSetsLayout; }

    /// @brief @see BaseShader#stages
    [[nodiscard]] std::vector<ShaderStage> stages() const { return _stages; };

    /// @brief @see BaseShader#entryPoint
    [[nodiscard]] const std::string& entryPoint(ShaderStage stage) const { return _entryPoints.at(stage); }

    /// @brief @see BaseShader#reload
    void reload();

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
    [[nodiscard]] static ShaderRef create(const ShaderInfo& shaderInfo);

private:
    std::unordered_map<ShaderStage, vk::ShaderModule> _shaderModules {}; ///< Shader modules mapped for stages.
    std::unordered_map<ShaderStage, std::string> _entryPoints {}; ///< Shader entry points.
    std::vector<DescriptorSetLayout> _descriptorSetsLayout {}; ///< Descriptor sets layout.
    std::vector<ShaderStage> _stages {}; ///< Shader stages.

    void cleanup();
};

} // namespace chronicle::internal::vulkan