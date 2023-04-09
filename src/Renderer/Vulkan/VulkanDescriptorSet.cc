#include "VulkanDescriptorSet.h"

#include "Vulkan.h"

namespace chronicle {

CHR_CONCRETE(VulkanDescriptorSet)

VulkanDescriptorSet::VulkanDescriptorSet(const vk::Device& device, const vk::PhysicalDevice& physicalDevice)
    : _device(device)
    , _physicalDevice(physicalDevice)
{
    CHRZONE_VULKAN

    // TODO: allocate on build with the right size
    // create a descriptor pool that will hold 10 uniform buffers
    std::vector<vk::DescriptorPoolSize> sizes
        = { { vk::DescriptorType::eUniformBuffer, 10 }, { vk::DescriptorType::eCombinedImageSampler, 10 } };

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(1);
    poolInfo.setPoolSizes(sizes);

    _descriptorPool = _device.createDescriptorPool(poolInfo, nullptr);
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    CHRZONE_VULKAN

    for (const auto& state : _descriptorSetsState) {
        if (state.type == vk::DescriptorType::eUniformBuffer) {
            _device.destroyBuffer(state.uniform.bufferInfo.buffer);
            _device.freeMemory(state.uniform.bufferMemory);
        }
    }

    if (_descriptorSetLayout)
        _device.destroyDescriptorSetLayout(_descriptorSetLayout);

    if (_descriptorPool)
        _device.destroyDescriptorPool(_descriptorPool);
}

void VulkanDescriptorSet::build()
{
    CHRZONE_VULKAN

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.setBindings(_layoutBindings);
    _descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.setDescriptorPool(_descriptorPool);
    allocInfo.setSetLayouts(_descriptorSetLayout);

    _descriptorSet = _device.allocateDescriptorSets(allocInfo)[0];

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

    _device.updateDescriptorSets(descriptorWrites, nullptr);
}

DescriptorSetRef VulkanDescriptorSet::create(const Renderer* renderer)
{
    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);

    return std::make_shared<ConcreteVulkanDescriptorSet>(vulkanRenderer->device(), vulkanRenderer->physicalDevice());
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