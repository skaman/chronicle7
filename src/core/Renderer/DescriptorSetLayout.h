// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Enums.h"

namespace chronicle {

/// @brief Structure specifying a descriptor set layout binding.
struct DescriptorSetLayoutBinding {
    /// @brief The binding number of this entry and corresponds to a resource of the same binding number in the shader
    ///        stages.
    uint32_t binding = 0;

    /// @brief Specify the binding number of this entry and corresponds to a resource of the same binding number in the
    ///        shader stages.
    DescriptorType descriptorType = DescriptorType::unknown;

    /// @brief The number of descriptors contained in the binding, accessed in a shader as an array.
    uint32_t descriptorCount = 0;

    /// @brief Specify which pipeline shader stages can access a resource for this binding.
    ShaderStage stageFlags = ShaderStage::none;
};

/// @brief Data structure that contain descriptor set informations read from the shader itself with spirv-reflect.
struct DescriptorSetLayout {
    uint32_t setNumber = 0; ///< Set number.
    std::vector<DescriptorSetLayoutBinding> bindings = {}; ///< Descriptor set layout bindings
};

} // namespace chronicle

template <> struct std::hash<chronicle::DescriptorSetLayoutBinding> {
    std::size_t operator()(const chronicle::DescriptorSetLayoutBinding& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.binding, data.descriptorType, data.descriptorCount, data.stageFlags);
        return h;
    }
};

template <> struct std::hash<chronicle::DescriptorSetLayout> {
    std::size_t operator()(const chronicle::DescriptorSetLayout& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.setNumber);
        for (const auto& binding : data.bindings) {
            std::hash_combine(h, binding);
        }
        return h;
    }
};