// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanDescriptorSet.h"

#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanDescriptorSet);

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy descriptor set");

    // clean data inside the binding info
    for (const auto& state : _descriptorSetsBindingInfo) {
        if (state.type == vk::DescriptorType::eUniformBuffer) {
            VulkanContext::device.destroyBuffer(state.uniform.bufferInfo.buffer);
            VulkanContext::device.freeMemory(state.uniform.bufferMemory);
        }
    }

    // clean the descriptor set layout
    if (_descriptorSetLayout)
        VulkanContext::device.destroyDescriptorSetLayout(_descriptorSetLayout);
}

void VulkanDescriptorSet::build()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Build descriptor set");

    assert(!_descriptorSet);

    // create the descriptor set layout
    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.setBindings(_layoutBindings);
    _descriptorSetLayout = VulkanContext::device.createDescriptorSetLayout(layoutInfo);

    // allocate the descriptor set
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.setDescriptorPool(VulkanContext::descriptorPool);
    allocInfo.setSetLayouts(_descriptorSetLayout);
    _descriptorSet = VulkanContext::device.allocateDescriptorSets(allocInfo)[0];

    // create the descriptors writes
    std::vector<vk::WriteDescriptorSet> descriptorWrites = {};
    descriptorWrites.reserve(_descriptorSetsBindingInfo.size());
    for (uint32_t i = 0; i < _descriptorSetsBindingInfo.size(); i++) {
        switch (_descriptorSetsBindingInfo[i].type) {
        case vk::DescriptorType::eUniformBuffer:
            descriptorWrites.push_back(createUniformWriteDescriptorSet(i, _descriptorSetsBindingInfo[i]));
            break;
        case vk::DescriptorType::eCombinedImageSampler:
            descriptorWrites.push_back(createCombinedImageSamplerWriteDescriptorSet(i, _descriptorSetsBindingInfo[i]));
            break;
        default:
            throw RendererError("Unsupported descriptor type");
        }
    }

    // update the descriptor sets
    VulkanContext::device.updateDescriptorSets(descriptorWrites, nullptr);
}

DescriptorSetRef VulkanDescriptorSet::create()
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanDescriptorSet>();
}

vk::WriteDescriptorSet VulkanDescriptorSet::createUniformWriteDescriptorSet(
    uint32_t index, const VulkanDescriptorSetBindingInfo& bindingInfo) const
{
    // create the write descriptor set
    vk::WriteDescriptorSet descriptorWrite = {};
    descriptorWrite.setDstSet(_descriptorSet);
    descriptorWrite.setDstBinding(index);
    descriptorWrite.setDstArrayElement(0);
    descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    descriptorWrite.setDescriptorCount(1);
    descriptorWrite.setBufferInfo(bindingInfo.uniform.bufferInfo);
    return descriptorWrite;
}

vk::WriteDescriptorSet VulkanDescriptorSet::createCombinedImageSamplerWriteDescriptorSet(
    uint32_t index, const VulkanDescriptorSetBindingInfo& bindingInfo) const
{
    // create the write descriptor set
    vk::WriteDescriptorSet descriptorWrite = {};
    descriptorWrite.setDstSet(_descriptorSet);
    descriptorWrite.setDstBinding(index);
    descriptorWrite.setDstArrayElement(0);
    descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    descriptorWrite.setDescriptorCount(1);
    descriptorWrite.setImageInfo(bindingInfo.combinedImageSampler.imageInfo);
    return descriptorWrite;
}

} // namespace chronicle