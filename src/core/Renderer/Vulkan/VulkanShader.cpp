// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShader.h"

#include "VulkanCommon.h"
#include "VulkanShaderCompiler.h"

namespace chronicle {

const int MAX_DESCRIPTOR_SETS = 4;

CHR_CONCRETE(VulkanShader);

VulkanShader::VulkanShader(const ShaderInfo& shaderInfo)
{
    const auto& shaderData = VulkanShaderCompiler::compile(
        { .filename = shaderInfo.filename, .macroDefinitions = shaderInfo.macroDefinitions });
    assert(shaderData.modules.size() > 0);

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
}

VulkanShader::~VulkanShader()
{
    // destroy shader modules
    for (const auto& [_, shaderModule] : _shaderModules) {
        VulkanContext::device.destroyShaderModule(shaderModule);
    }
}

ShaderRef VulkanShader::create(const ShaderInfo& shaderInfo)
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanShader>(shaderInfo);
}

} // namespace chronicle