// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseDescriptorSet.h"

#include "VulkanCommon.h"
#include "VulkanEnums.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

namespace chronicle {

/// @brief Binding data for uniform buffer.
struct UniformStateBindingData {
    vk::DeviceMemory bufferMemory {}; ///< Device memory that contain the data.
    vk::DescriptorBufferInfo bufferInfo {}; ///< Descriptor buffer informations.
};

/// @brief Binding data for a sampler.
struct CombinedImageSamplerBindingData {
    vk::DescriptorImageInfo imageInfo {}; ///< Descriptor image informations.
};

/// @brief Binding informations for a descriptor.
struct VulkanDescriptorSetBindingInfo {
    vk::DescriptorType type {}; ///< Descriptor type.

    union {
        UniformStateBindingData uniform; ///< Uniform buffer info.
        CombinedImageSamplerBindingData combinedImageSampler; ///< Sampler info.
    };
};

/// @brief Vulkan implementation for @ref BaseDescriptorSet
class VulkanDescriptorSet : public BaseDescriptorSet<VulkanDescriptorSet>, private NonCopyable<VulkanDescriptorSet> {
protected:
    /// @brief Default constructor.
    /// @param name Debug name.
    explicit VulkanDescriptorSet(const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanDescriptorSet();

    /// @brief @see BaseDescriptorSet#name
    [[nodiscard]] std::string name() const { return _name; }

    /// @brief @see BaseDescriptorSet#descriptorSetId
    [[nodiscard]] DescriptorSetId descriptorSetId() const { return _descriptorSet; }

    /// @brief @see BaseDescriptorSet#addUniform
    template <class T> void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        assert(stage != ShaderStage::none);

        // create the descriptor set layout binding
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.setBinding(static_cast<uint32_t>(_layoutBindings.size()));
        layoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        layoutBinding.setDescriptorCount(1);
        layoutBinding.setStageFlags(VulkanEnums::shaderStageToVulkan(stage));
        _layoutBindings.push_back(layoutBinding);

        // create a buffer that is visible to the host, so it can be updated for every frame.
        uint32_t bufferSize = sizeof(T);

        assert(bufferSize > 0);

        void* bufferMapped;
        auto [bufferMemory, buffer] = VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        assert(buffer);
        assert(bufferMemory);

        bufferMapped = VulkanContext::device.mapMemory(bufferMemory, 0, bufferSize);

        assert(bufferMapped);

        _buffersMapped[id] = bufferMapped;

        // create the descriptor buffer informations.
        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.setBuffer(buffer);
        bufferInfo.setOffset(0);
        bufferInfo.setRange(bufferSize);

        // create and add the descriptor binding informations
        VulkanDescriptorSetBindingInfo descriptorSetBinding = { .type = vk::DescriptorType::eUniformBuffer,
            .uniform = { .bufferMemory = bufferMemory, .bufferInfo = bufferInfo } };
        _descriptorSetsBindingInfo.push_back(descriptorSetBinding);
    }

    /// @brief @see BaseDescriptorSet#addSampler
    void addSampler(ShaderStage stage, const TextureRef texture)
    {
        assert(stage != ShaderStage::none);
        assert(texture);

        // create the descriptor set layout binding
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.setBinding(static_cast<uint32_t>(_layoutBindings.size()));
        layoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        layoutBinding.setDescriptorCount(1);
        layoutBinding.setStageFlags(VulkanEnums::shaderStageToVulkan(stage));
        _layoutBindings.push_back(layoutBinding);

        // create the descriptor image informations.
        vk::DescriptorImageInfo imageInfo {};
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfo.setImageView(texture->textureId());
        imageInfo.setSampler(texture->samplerId());

        // create and add the descriptor binding informations
        VulkanDescriptorSetBindingInfo descriptorSetBinding
            = { .type = vk::DescriptorType::eCombinedImageSampler, .combinedImageSampler = { .imageInfo = imageInfo } };
        _descriptorSetsBindingInfo.push_back(descriptorSetBinding);
    }

    /// @brief @see BaseDescriptorSet#setUniform
    template <class T> void setUniform(entt::hashed_string::hash_type id, const T& data)
    {
        std::memcpy(_buffersMapped[id], &data, sizeof(T));
    }

    /// @brief @see BaseDescriptorSet#build
    void build();

    /// @brief @see BaseDescriptorSet#create
    /// @param name Name.
    [[nodiscard]] static DescriptorSetRef create(const std::string& name);

private:
    std::string _name {}; ///< Name.

    vk::DescriptorSet _descriptorSet {}; ///< Descriptor set handle.
    vk::DescriptorSetLayout _descriptorSetLayout {}; ///< Descriptor set layout.

    std::vector<vk::DescriptorSetLayoutBinding> _layoutBindings {}; ///< Layout bindings.
    std::vector<VulkanDescriptorSetBindingInfo> _descriptorSetsBindingInfo {}; ///< Descriptor sets binding info.

    std::unordered_map<entt::hashed_string::hash_type, void*> _buffersMapped {}; ///< Map for uniform buffers memory.

    /// @brief Create a write descriptor set for a uniform.
    /// @param index Descriptor set index.
    /// @param bindingInfo Binding informations for the descriptor.
    /// @return Write descriptor set.
    [[nodiscard]] vk::WriteDescriptorSet createUniformWriteDescriptorSet(
        uint32_t index, const VulkanDescriptorSetBindingInfo& bindingInfo) const;

    /// @brief Create a write descriptor set for a sampler.
    /// @param index Descriptor set index.
    /// @param bindingInfo Binding informations for the descriptor.
    /// @return Write descriptor set.
    [[nodiscard]] vk::WriteDescriptorSet createCombinedImageSamplerWriteDescriptorSet(
        uint32_t index, const VulkanDescriptorSetBindingInfo& bindingInfo) const;
};

} // namespace chronicle