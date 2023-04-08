#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/Image.h"
#include "Renderer/RendererError.h"

#include "VulkanBuffer.h"
#include "VulkanCommon.h"

namespace chronicle {

struct VulkanDescriptorSetUniformState {
    vk::DeviceMemory bufferMemory;
    vk::DescriptorBufferInfo bufferInfo;
};

struct VulkanDescriptorSetCombinedImageSamplerState {
    vk::DescriptorImageInfo imageInfo;
};

struct VulkanDescriptorSetState {
    vk::DescriptorType type;

    union {
        VulkanDescriptorSetUniformState uniform;
        VulkanDescriptorSetCombinedImageSamplerState combinedImageSampler;
    };
};

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

        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.setBuffer(buffer);
        bufferInfo.setOffset(0);
        bufferInfo.setRange(bufferSize);

        VulkanDescriptorSetState descriptorSetState = { .type = vk::DescriptorType::eUniformBuffer,
            .uniform = { .bufferMemory = bufferMemory, .bufferInfo = bufferInfo } };

        _descriptorSetsState.push_back(descriptorSetState);
        _buffersMapped[id] = bufferMapped;
    }

    void addSampler(ShaderStage stage, const std::shared_ptr<Image> image)
    {
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.setBinding(static_cast<uint32_t>(_layoutBindings.size()));
        layoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        layoutBinding.setDescriptorCount(1);
        layoutBinding.setStageFlags(shaderStageToVulkan(stage));
        _layoutBindings.push_back(layoutBinding);

        vk::DescriptorImageInfo imageInfo {};
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfo.setImageView(image->native().imageView());
        imageInfo.setSampler(image->native().sampler());

        VulkanDescriptorSetState descriptorSetState
            = { .type = vk::DescriptorType::eCombinedImageSampler, .combinedImageSampler = { .imageInfo = imageInfo } };

        _descriptorSetsState.push_back(descriptorSetState);
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
    std::vector<VulkanDescriptorSetState> _descriptorSetsState = {};

    std::unordered_map<entt::hashed_string::hash_type, void*> _buffersMapped = {};

    [[nodiscard]] vk::WriteDescriptorSet createUniformWriteDescriptorSet(
        uint32_t index, const VulkanDescriptorSetState& state) const;
    [[nodiscard]] vk::WriteDescriptorSet createCombinedImageSamplerWriteDescriptorSet(
        uint32_t index, const VulkanDescriptorSetState& state) const;
};

} // namespace chronicle