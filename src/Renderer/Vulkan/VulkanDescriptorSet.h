#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/RendererError.h"

#include "VulkanBuffer.h"
#include "VulkanCommon.h"

namespace chronicle {

class VulkanDescriptorSet {
public:
    explicit VulkanDescriptorSet(const vk::Device& device, const vk::PhysicalDevice& physicalDevice);
    ~VulkanDescriptorSet();

    template <class T> void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.setBinding(static_cast<uint32_t>(_layoutBindings.size()));
        layoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        layoutBinding.setDescriptorCount(1);
        layoutBinding.setStageFlags(shaderStageToVulkan(stage));
        _layoutBindings.push_back(layoutBinding);

        uint32_t bufferSize = sizeof(T);

        vk::Buffer buffer;
        vk::DeviceMemory bufferMemory;
        void* bufferMapped;

        VulkanBuffer::create(_device, _physicalDevice, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMemory);

        bufferMapped = _device.mapMemory(bufferMemory, 0, bufferSize);

        _buffers.push_back(buffer);
        _buffersMemory.push_back(bufferMemory);
        _buffersMapped[id] = bufferMapped;
        _buffersSize.push_back(bufferSize);
    }

    template <class T> void setUniform(entt::hashed_string::hash_type id, const T& data)
    {
        memcpy(_buffersMapped[id], &data, sizeof(T));
    }

    void build();

    // internal
    [[nodiscard]] inline const std::vector<vk::DescriptorSetLayoutBinding>& layoutBindings() const
    {
        return _layoutBindings;
    }
    [[nodiscard]] inline const vk::DescriptorSet& descriptorSet() const { return _descriptorSet; }

private:
    vk::Device _device;
    vk::PhysicalDevice _physicalDevice;

    vk::DescriptorPool _descriptorPool;
    vk::DescriptorSet _descriptorSet;
    vk::DescriptorSetLayout _descriptorSetLayout;

    std::vector<vk::DescriptorSetLayoutBinding> _layoutBindings = {};

    std::vector<vk::Buffer> _buffers = {};
    std::vector<vk::DeviceMemory> _buffersMemory = {};
    std::unordered_map<entt::hashed_string::hash_type, void*> _buffersMapped = {};
    std::vector<uint32_t> _buffersSize = {};
};

} // namespace chronicle