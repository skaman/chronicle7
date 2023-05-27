// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanDescriptorSetOld.h"

#include "VulkanGC.h"
#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanDescriptorSet);

VulkanDescriptorSet::VulkanDescriptorSet(const std::string& name)
    : _name(name)
{
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Destroy descriptor set");

    // clean data inside the binding info
    for (const auto& state : _descriptorSetsBindingInfo) {
        if (state.type == vk::DescriptorType::eUniformBuffer) {
            VulkanGC::add(state.uniform.bufferInfo.buffer);
            VulkanGC::add(state.uniform.bufferMemory);
        }
    }

    // clean the descriptor set layout
    if (_descriptorSetLayout)
        VulkanContext::device.destroyDescriptorSetLayout(_descriptorSetLayout);
}

void VulkanDescriptorSet::build()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Build descriptor set");

    assert(!_descriptorSet);
    assert(!_descriptorSetsBindingInfo.empty());

    // create the descriptor set layout
    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.setBindings(_layoutBindings);
    _descriptorSetLayout = VulkanContext::device.createDescriptorSetLayout(layoutInfo);

    assert(_descriptorSetLayout);
    assert(VulkanContext::descriptorPool);

    // allocate the descriptor set
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.setDescriptorPool(VulkanContext::descriptorPool);
    allocInfo.setSetLayouts(_descriptorSetLayout);
    _descriptorSet = VulkanContext::device.allocateDescriptorSets(allocInfo)[0];

    assert(_descriptorSet);

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

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::setDebugObjectName(_descriptorSet, _name);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

DescriptorSetRef VulkanDescriptorSet::create(const std::string& _name)
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanDescriptorSet>(_name);
}

vk::WriteDescriptorSet VulkanDescriptorSet::createUniformWriteDescriptorSet(
    uint32_t index, const VulkanDescriptorSetBindingInfo& bindingInfo) const
{
    assert(_descriptorSet);

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
    assert(_descriptorSet);

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