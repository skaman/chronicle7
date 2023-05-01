// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanMaterial.h"

#include "VulkanDescriptorSet.h"

using namespace entt::literals;

namespace chronicle {

CHR_CONCRETE(VulkanMaterial);

VulkanMaterial::VulkanMaterial(const char* debugName) { }

void VulkanMaterial::build()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Build material");

    const char* debugName = nullptr;

    _descriptorSet = DescriptorSet::create(debugName);
    _descriptorSet->addUniform<MaterialUBO>("ubo"_hs, ShaderStage::vertex);
    if (_baseColorTexture) {
        CHRLOG_DEBUG("Material: found base color texture");
        _descriptorSet->addSampler(ShaderStage::fragment, _baseColorTexture);
    }
    if (_metallicRoughnessTexture) {
        CHRLOG_DEBUG("Material: found metallic roughness texture");
        _descriptorSet->addSampler(ShaderStage::fragment, _metallicRoughnessTexture);
    }
    if (_normalTexture) {
        CHRLOG_DEBUG("Material: found normal texture");
        _descriptorSet->addSampler(ShaderStage::fragment, _normalTexture);
    }
    if (_occlusionTexture) {
        CHRLOG_DEBUG("Material: found occlusion texture");
        _descriptorSet->addSampler(ShaderStage::fragment, _occlusionTexture);
    }
    if (_emissiveTexture) {
        CHRLOG_DEBUG("Material: found emissive texture");
        _descriptorSet->addSampler(ShaderStage::fragment, _emissiveTexture);
    }
    _descriptorSet->build();
}

MaterialRef VulkanMaterial::create(const char* debugName)
{
    return std::make_shared<ConcreteVulkanMaterial>(debugName);
}

} // namespace chronicle