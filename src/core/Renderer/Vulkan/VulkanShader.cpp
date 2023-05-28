// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShader.h"

#include "VulkanCommon.h"
#include "VulkanShaderCompiler.h"

#include "Storage/StorageContext.h"

namespace chronicle::internal::vulkan {

const int MAX_DESCRIPTOR_SETS = 4;

CHR_CONCRETE(VulkanShader);

VulkanShader::VulkanShader(const ShaderInfo& shaderInfo)
    : BaseShader(shaderInfo)
{
    reload();
}

VulkanShader::~VulkanShader() { cleanup(); }

ShaderRef VulkanShader::create(const ShaderInfo& shaderInfo)
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanShader>(shaderInfo);
}

void vulkan::VulkanShader::reload()
{
    CHRLOG_DEBUG("Shader load: {} ({})", _shaderInfo.filename, join(_shaderInfo.macroDefinitions));

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        const auto& shaderData = VulkanShaderCompiler::compile(
            { .filename = _shaderInfo.filename, .macroDefinitions = _shaderInfo.macroDefinitions });

        cleanup();

        _stages.reserve(shaderData.modules.size());
        _shaderModules.reserve(shaderData.modules.size());
        _entryPoints.reserve(shaderData.modules.size());
        for (const auto& [stage, shaderModule] : shaderData.modules) {
            // create shader module
            _shaderModules[stage] = VulkanContext::device.createShaderModule({ vk::ShaderModuleCreateFlags(),
                shaderModule.spirvBinary.size() * sizeof(uint32_t), shaderModule.spirvBinary.data() });
            _entryPoints[stage] = shaderModule.entryPoint;
            _stages.push_back(stage);
        }
    } catch (const std::exception& e) {
        CHRLOG_ERROR("Error loading shader: {}", e.what());
        return;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    CHRLOG_DEBUG("Shader loaded in {} ms: {} ({})", duration.count() / 1000.0f, _shaderInfo.filename,
        join(_shaderInfo.macroDefinitions));
}

void vulkan::VulkanShader::cleanup()
{
    // destroy shader modules
    for (const auto& [_, shaderModule] : _shaderModules) {
        VulkanContext::device.destroyShaderModule(shaderModule);
    }
    _shaderModules.clear();
    _entryPoints.clear();
    _descriptorSetsLayout.clear();
    _stages.clear();
}

} // namespace chronicle::internal::vulkan