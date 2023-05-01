// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShader.h"

#include "VulkanCommon.h"

namespace chronicle {

const int MAX_DESCRIPTOR_SETS = 4;

CHR_CONCRETE(VulkanShader);

VulkanShader::VulkanShader(const std::unordered_map<ShaderStage, std::vector<uint8_t>>& codes,
    const std::unordered_map<ShaderStage, std::string>& entryPoints)
    : _entryPoints(entryPoints)
{
    assert(codes.size() > 0);
    assert(codes.size() == entryPoints.size());

    _stages.reserve(codes.size());
    for (const auto& [stage, code] : codes) {
        // create shader module
        _shaderModules[stage] = VulkanContext::device.createShaderModule(
            { vk::ShaderModuleCreateFlags(), code.size(), std::bit_cast<const uint32_t*>(code.data()) });
        _stages.push_back(stage);
    }

    // prepare the sets map
    // set number -> binding id -> layout binding
    std::map<uint32_t, std::map<uint32_t, DescriptorSetLayoutBinding>> sets = {};

    // fill the set
    for (const auto& [_, code] : codes) {
        auto descriptorSetsLayoutData = getDescriptorSetsLayout(code);
        for (const auto& descriptorSetLayoutData : descriptorSetsLayoutData) {
            // if set doesn't exist, create it
            if (!sets.contains(descriptorSetLayoutData.setNumber))
                sets[descriptorSetLayoutData.setNumber] = {};

            for (const auto& descriptorSetLayoutBinding : descriptorSetLayoutData.bindings) {
                auto& descriptorSet = sets[descriptorSetLayoutData.setNumber];

                // if binding doesn't exist, create it
                if (!descriptorSet.contains(descriptorSetLayoutBinding.binding)) {
                    descriptorSet[descriptorSetLayoutBinding.binding] = descriptorSetLayoutBinding;
                } else {
                    auto& layoutBinding = descriptorSet[descriptorSetLayoutBinding.binding];
                    layoutBinding.stageFlags |= descriptorSetLayoutBinding.stageFlags;

                    // with HLSL combined sampler are composed from a simple sampler and a sampled image.
                    if ((layoutBinding.descriptorType == DescriptorType::sampler
                            && descriptorSetLayoutBinding.descriptorType == DescriptorType::sampledImage)
                        || (layoutBinding.descriptorType == DescriptorType::sampledImage
                            && descriptorSetLayoutBinding.descriptorType == DescriptorType::sampler)) {
                        layoutBinding.descriptorType = DescriptorType::combinedImageSampler;
                    }
                }
            }
        }
    }

    // reserve descriptor sets layout memory
    _descriptorSetsLayout.resize(MAX_DESCRIPTOR_SETS);

    // create the descriptor set layouts
    for (uint32_t i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
        std::vector<DescriptorSetLayoutBinding> bindings = {};
        if (sets.contains(i)) {
            bindings.reserve(sets[i].size());
            for (auto const& [bindingId, binding] : sets[i]) {
                bindings.push_back(binding);
            }
        }

        // create the descriptor set layout
        _descriptorSetsLayout[i] = DescriptorSetLayout { .setNumber = i, .bindings = bindings };
    }
}

VulkanShader::~VulkanShader()
{
    // destroy shader modules
    for (const auto& [_, shaderModule] : _shaderModules) {
        VulkanContext::device.destroyShaderModule(shaderModule);
    }
}

ShaderRef VulkanShader::create(const std::unordered_map<ShaderStage, std::vector<uint8_t>>& codes,
    const std::unordered_map<ShaderStage, std::string>& entryPoints)
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanShader>(codes, entryPoints);
}

std::vector<DescriptorSetLayout> VulkanShader::getDescriptorSetsLayout(const std::vector<uint8_t>& code)
{
    // create the spirv-reflect shader module
    SpvReflectShaderModule shaderModule = {};
    SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &shaderModule);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // count the descriptor sets
    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&shaderModule, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // enumerate the descriptor sets
    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = spvReflectEnumerateDescriptorSets(&shaderModule, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // parse the sets and prepare the descriptor set layout data
    std::vector<DescriptorSetLayout> layoutSets(sets.size(), DescriptorSetLayout {});
    for (size_t setIndex = 0; setIndex < sets.size(); ++setIndex) {
        const SpvReflectDescriptorSet& setRefl = *(sets[setIndex]);
        DescriptorSetLayout& layout = layoutSets[setIndex];
        layout.bindings.resize(setRefl.binding_count);
        for (uint32_t bindingIndex = 0; bindingIndex < setRefl.binding_count; ++bindingIndex) {
            const SpvReflectDescriptorBinding& bindingRefl = *(setRefl.bindings[bindingIndex]);
            DescriptorSetLayoutBinding& layoutBinding = layout.bindings[bindingIndex];
            layoutBinding.binding = bindingRefl.binding;
            auto descriptorType = getDescriptorType(bindingRefl.descriptor_type);
            auto shaderStage = getShaderStage(shaderModule.shader_stage);
            if (descriptorType == DescriptorType::unknown || shaderStage == ShaderStage::none) {
                continue;
            }
            layoutBinding.descriptorType = descriptorType;
            layoutBinding.descriptorCount = 1;
            for (uint32_t dimensionIndex = 0; dimensionIndex < bindingRefl.array.dims_count; ++dimensionIndex) {
                layoutBinding.descriptorCount *= bindingRefl.array.dims[dimensionIndex];
            }
            layoutBinding.stageFlags = shaderStage;
        }
        layout.setNumber = setRefl.set;
    }

    // return the descriptor set layout data
    return layoutSets;
}

DescriptorType VulkanShader::getDescriptorType(SpvReflectDescriptorType spvDescriptorType)
{
    switch (spvDescriptorType) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
        return DescriptorType::sampler;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return DescriptorType::combinedImageSampler;
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return DescriptorType::sampledImage;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return DescriptorType::storageImage;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return DescriptorType::uniformTexelBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return DescriptorType::storageTexelBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return DescriptorType::uniformBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return DescriptorType::storageBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return DescriptorType::uniformBufferDynamic;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return DescriptorType::storageBufferDynamic;
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return DescriptorType::inputAttachment;
    case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        return DescriptorType::accelerationStructure;
    default:
        return DescriptorType::unknown;
    }
}

ShaderStage VulkanShader::getShaderStage(SpvReflectShaderStageFlagBits spvShaderStage)
{
    auto shaderStage = ShaderStage::none;

    if (spvShaderStage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
        shaderStage |= ShaderStage::vertex;
    if (spvShaderStage & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT)
        shaderStage |= ShaderStage::fragment;
    if (spvShaderStage & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT)
        shaderStage |= ShaderStage::compute;

    return shaderStage;
}

} // namespace chronicle