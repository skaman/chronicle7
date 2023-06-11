// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShaderModule.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

#include <regex>
#include <shaderc/shaderc.hpp>

namespace chronicle::graphics::internal::vulkan
{

inline shaderc_shader_kind convertShaderStage(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::eVertex:
        return shaderc_vertex_shader;
    case ShaderStage::eFragment:
        return shaderc_fragment_shader;
    case ShaderStage::eCompute:
        return shaderc_compute_shader;
    default:
        throw ShaderModuleError("Unsupported shader stage");
    }
}

VulkanShaderModule::VulkanShaderModule(std::shared_ptr<VulkanDevice> device,
                                       const ShaderModuleDescriptor &shaderModuleDescriptor)
    : ShaderModule(shaderModuleDescriptor), _device(device)
{
    shaderc::Compiler spirvCompiler = {};
    shaderc::CompileOptions spirvOptions = {};

    spirvOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
    spirvOptions.SetSourceLanguage(shaderc_source_language_glsl);

    switch (descriptor().optimizationLevel)
    {
    case ShaderOptimizationLevel::ePerformance:
        spirvOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
        break;
    case ShaderOptimizationLevel::eSize:
        spirvOptions.SetOptimizationLevel(shaderc_optimization_level_size);
        break;
    case ShaderOptimizationLevel::eZero:
        spirvOptions.SetOptimizationLevel(shaderc_optimization_level_zero);
        break;
    }

    for (const auto &macroDefinition : descriptor().macroDefinitions)
    {
        spirvOptions.AddMacroDefinition(macroDefinition);
    }

    auto spirvCompilerResult = spirvCompiler.CompileGlslToSpv(
        descriptor().sourceCode, convertShaderStage(descriptor().shaderStage), descriptor().name.c_str(), spirvOptions);

    std::stringstream streamMessage(spirvCompilerResult.GetErrorMessage());
    std::string segment;
    const std::regex errorRegex(".*:\\d+: error:.*");
    const std::regex warningRegex(".*:\\d+: warning:.*");

    while (std::getline(streamMessage, segment, '\n'))
    {
        if (std::regex_match(segment, errorRegex))
        {
            CHRLOG_ERROR("{}", segment);
        }
        else if (std::regex_match(segment, warningRegex))
        {
            CHRLOG_WARN("{}", segment);
        }
        else
        {
            CHRLOG_INFO("{}", segment);
        }
    }

    if (spirvCompilerResult.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        throw ShaderModuleError(fmt::format("Can't compile shader {}", descriptor().name));
    }

    auto spirvBinary = std::vector<uint32_t>(spirvCompilerResult.begin(), spirvCompilerResult.end());

    try
    {
        _vulkanShaderModule = _device->vulkanLogicalDevice().createShaderModule(
            {vk::ShaderModuleCreateFlags(), spirvBinary.size() * sizeof(uint32_t), spirvBinary.data()});
    }
    catch (const vk::Error &error)
    {
        throw ShaderModuleError(fmt::format("Can't create shader: {}", error.what()));
    }
}

VulkanShaderModule::~VulkanShaderModule()
{
    _device->vulkanLogicalDevice().destroyShaderModule(_vulkanShaderModule);
}

} // namespace chronicle::graphics::internal::vulkan