#include "VulkanDescriptorSet.h"

namespace chronicle {

VulkanDescriptorSet::VulkanDescriptorSet(const vk::Device& device, const vk::PhysicalDevice& physicalDevice)
    : _device(device)
    , _physicalDevice(physicalDevice)
{
    // TODO: allocate on build with the right size
    // create a descriptor pool that will hold 10 uniform buffers
    std::vector<vk::DescriptorPoolSize> sizes = { { vk::DescriptorType::eUniformBuffer, 10 } };

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(1);
    poolInfo.setPoolSizes(sizes);

    _descriptorPool = _device.createDescriptorPool(poolInfo, nullptr);
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    for (const auto& buffer : _buffers)
        _device.destroyBuffer(buffer);

    for (const auto& bufferMemory : _buffersMemory)
        _device.freeMemory(bufferMemory);

    if (_descriptorSetLayout)
        _device.destroyDescriptorSetLayout(_descriptorSetLayout);

    _device.destroyDescriptorPool(_descriptorPool);
}

void VulkanDescriptorSet::build()
{
    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.setBindings(_layoutBindings);
    _descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.setDescriptorPool(_descriptorPool);
    allocInfo.setSetLayouts(_descriptorSetLayout);

    _descriptorSet = _device.allocateDescriptorSets(allocInfo)[0];

    for (size_t i = 0; i < _layoutBindings.size(); i++) {
        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.setBuffer(_buffers[i]);
        bufferInfo.setOffset(0);
        bufferInfo.setRange(_buffersSize[i]);

        vk::WriteDescriptorSet descriptorWrite = {};
        descriptorWrite.setDstSet(_descriptorSet);
        descriptorWrite.setDstBinding(static_cast<uint32_t>(i));
        descriptorWrite.setDstArrayElement(0);
        descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        descriptorWrite.setDescriptorCount(1);
        descriptorWrite.setBufferInfo(bufferInfo);

        _device.updateDescriptorSets(descriptorWrite, nullptr);
    }
}

} // namespace chronicle