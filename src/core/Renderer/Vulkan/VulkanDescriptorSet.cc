// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanDescriptorSet.h"

#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanDescriptorSet);

VulkanDescriptorSet::VulkanDescriptorSet()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create descriptor set");

    // TODO: allocate on build with the right size
    // create a descriptor pool that will hold 10 uniform buffers
    std::vector<vk::DescriptorPoolSize> sizes
        = { { vk::DescriptorType::eUniformBuffer, 10 }, { vk::DescriptorType::eCombinedImageSampler, 10 } };

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(1);
    poolInfo.setPoolSizes(sizes);

    _descriptorPool = VulkanContext::device.createDescriptorPool(poolInfo, nullptr);
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy descriptor set");

    for (const auto& state : _descriptorSetsState) {
        if (state.type == vk::DescriptorType::eUniformBuffer) {
            VulkanContext::device.destroyBuffer(state.uniform.bufferInfo.buffer);
            VulkanContext::device.freeMemory(state.uniform.bufferMemory);
        }
    }

    if (_descriptorSetLayout)
        VulkanContext::device.destroyDescriptorSetLayout(_descriptorSetLayout);

    if (_descriptorPool)
        VulkanContext::device.destroyDescriptorPool(_descriptorPool);
}

void VulkanDescriptorSet::build()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Build descriptor set");

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.setBindings(_layoutBindings);
    _descriptorSetLayout = VulkanContext::device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.setDescriptorPool(_descriptorPool);
    allocInfo.setSetLayouts(_descriptorSetLayout);

    _descriptorSet = VulkanContext::device.allocateDescriptorSets(allocInfo)[0];

    std::vector<vk::WriteDescriptorSet> descriptorWrites = {};
    descriptorWrites.reserve(_descriptorSetsState.size());

    for (uint32_t i = 0; i < _descriptorSetsState.size(); i++) {
        switch (_descriptorSetsState[i].type) {
        case vk::DescriptorType::eUniformBuffer:
            descriptorWrites.push_back(createUniformWriteDescriptorSet(i, _descriptorSetsState[i]));
            break;
        case vk::DescriptorType::eCombinedImageSampler:
            descriptorWrites.push_back(createCombinedImageSamplerWriteDescriptorSet(i, _descriptorSetsState[i]));
            break;
        default:
            throw RendererError("Unsupported descriptor type");
        }
    }

    VulkanContext::device.updateDescriptorSets(descriptorWrites, nullptr);
}

DescriptorSetRef VulkanDescriptorSet::create()
{
    return std::make_shared<ConcreteVulkanDescriptorSet>();
}

vk::WriteDescriptorSet VulkanDescriptorSet::createUniformWriteDescriptorSet(
    uint32_t index, const VulkanDescriptorSetState& state) const
{
    vk::WriteDescriptorSet descriptorWrite = {};
    descriptorWrite.setDstSet(_descriptorSet);
    descriptorWrite.setDstBinding(index);
    descriptorWrite.setDstArrayElement(0);
    descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    descriptorWrite.setDescriptorCount(1);
    descriptorWrite.setBufferInfo(state.uniform.bufferInfo);
    return descriptorWrite;
}

vk::WriteDescriptorSet VulkanDescriptorSet::createCombinedImageSamplerWriteDescriptorSet(
    uint32_t index, const VulkanDescriptorSetState& state) const
{
    vk::WriteDescriptorSet descriptorWrite = {};
    descriptorWrite.setDstSet(_descriptorSet);
    descriptorWrite.setDstBinding(index);
    descriptorWrite.setDstArrayElement(0);
    descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    descriptorWrite.setDescriptorCount(1);
    descriptorWrite.setImageInfo(state.combinedImageSampler.imageInfo);
    return descriptorWrite;
}

} // namespace chronicle