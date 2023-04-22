// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/DescriptorSetI.h"

#include "VulkanCommon.h"
#include "VulkanEnums.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

namespace chronicle {

/// @brief Binding data for uniform buffer.
struct UniformStateBindingData {
    vk::DeviceMemory bufferMemory; ///< Device memory that contain the data.
    vk::DescriptorBufferInfo bufferInfo; ///< Descriptor buffer informations.
};

/// @brief Binding data for a sampler.
struct CombinedImageSamplerBindingData {
    vk::DescriptorImageInfo imageInfo; ///< Descriptor image informations.
};

/// @brief Binding informations for a descriptor.
struct VulkanDescriptorSetBindingInfo {
    vk::DescriptorType type; ///< Descriptor type.

    union {
        UniformStateBindingData uniform; ///< Uniform buffer info.
        CombinedImageSamplerBindingData combinedImageSampler; ///< Sampler info.
    };
};

/// @brief Vulkan implementation for @ref DescriptorSetI
class VulkanDescriptorSet : public DescriptorSetI<VulkanDescriptorSet>, private NonCopyable<VulkanDescriptorSet> {
protected:
    /// @brief Default constructor.
    /// @param debugName Debug name.
    explicit VulkanDescriptorSet(const char* debugName);

public:
    /// @brief Destructor.
    ~VulkanDescriptorSet();

    /// @brief @see DescriptorSetI#addUniform
    template <class T> void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        // create the descriptor set layout binding
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.setBinding(static_cast<uint32_t>(_layoutBindings.size()));
        layoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        layoutBinding.setDescriptorCount(1);
        layoutBinding.setStageFlags(VulkanEnums::shaderStageToVulkan(stage));
        _layoutBindings.push_back(layoutBinding);

        // create a buffer that is visible to the host, so it can be updated for every frame.
        uint32_t bufferSize = sizeof(T);
        void* bufferMapped;
        auto [bufferMemory, buffer] = VulkanUtils::createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        bufferMapped = VulkanContext::device.mapMemory(bufferMemory, 0, bufferSize);
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

    /// @brief @see DescriptorSetI#addSampler
    void addSampler(ShaderStage stage, const TextureRef texture)
    {
        // create the descriptor set layout binding
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.setBinding(static_cast<uint32_t>(_layoutBindings.size()));
        layoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        layoutBinding.setDescriptorCount(1);
        layoutBinding.setStageFlags(VulkanEnums::shaderStageToVulkan(stage));
        _layoutBindings.push_back(layoutBinding);

        // cast the texture to a vulkan texture
        const auto vulkanTexture = static_cast<const VulkanTexture*>(texture.get());

        // create the descriptor image informations.
        vk::DescriptorImageInfo imageInfo {};
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfo.setImageView(vulkanTexture->imageView());
        imageInfo.setSampler(vulkanTexture->sampler());

        // create and add the descriptor binding informations
        VulkanDescriptorSetBindingInfo descriptorSetBinding
            = { .type = vk::DescriptorType::eCombinedImageSampler, .combinedImageSampler = { .imageInfo = imageInfo } };
        _descriptorSetsBindingInfo.push_back(descriptorSetBinding);
    }

    /// @brief @see DescriptorSetI#setUniform
    template <class T> void setUniform(entt::hashed_string::hash_type id, const T& data)
    {
        memcpy(_buffersMapped[id], &data, sizeof(T));
    }

    /// @brief @see DescriptorSetI#build
    void build();

    /// @brief Get the vulkan layout bindings for the descriptor set.
    /// @return Layout bindings.
    [[nodiscard]] const std::vector<vk::DescriptorSetLayoutBinding>& layoutBindings() const { return _layoutBindings; }

    /// @brief Get the vulkan handle for the descriptor set.
    /// @return Vulkan handle.
    [[nodiscard]] const vk::DescriptorSet& descriptorSet() const { return _descriptorSet; }

    /// @brief @see DescriptorSetI#create
    /// @param debugName Debug name.
    [[nodiscard]] static DescriptorSetRef create(const char* debugName);

private:
    std::string _debugName; ///< Debug name.

    vk::DescriptorSet _descriptorSet = nullptr; ///< Descriptor set handle.
    vk::DescriptorSetLayout _descriptorSetLayout = nullptr; ///< Descriptor set layout.

    std::vector<vk::DescriptorSetLayoutBinding> _layoutBindings = {}; ///< Layout bindings.
    std::vector<VulkanDescriptorSetBindingInfo> _descriptorSetsBindingInfo = {}; ///< Descriptor sets binding info.

    std::unordered_map<entt::hashed_string::hash_type, void*> _buffersMapped = {}; ///< Map for uniform buffers memory.

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