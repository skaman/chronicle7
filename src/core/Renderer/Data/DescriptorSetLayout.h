// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common/Enums.h"

namespace chronicle {

struct DescriptorSetLayoutUniformMember {
    std::string name {};
    Type type { Type::unknown };
    uint32_t arraySize { 0 };

    [[nodiscard]] bool operator==(const DescriptorSetLayoutUniformMember& other) const
    {
        return name == other.name && type == other.type && arraySize == other.arraySize;
    }
};

struct DescriptorSetLayoutBinding {
    /// @brief The binding number of this entry and corresponds to a resource of the same binding number in the shader
    ///        stages.
    uint32_t binding { 0 };

    uint32_t descriptorSet { 0 };

    /// @brief Specify the binding number of this entry and corresponds to a resource of the same binding number in the
    ///        shader stages.
    DescriptorType descriptorType { DescriptorType::unknown };

    std::string name {};

    std::vector<DescriptorSetLayoutUniformMember> uniformMembers {};

    size_t uniformSize { 0 };

    uint32_t arraySize { 0 };

    ShaderStage stages { ShaderStage::none };

    [[nodiscard]] bool checkCompatibility(const DescriptorSetLayoutBinding& other) const
    {
        return name == other.name && uniformMembers == other.uniformMembers && uniformSize == other.uniformSize
            && arraySize == other.arraySize;
    }
};

// @brief Data structure that contain descriptor set informations read from the shader itself with spirv-reflect.
struct DescriptorSetLayout {
    uint32_t setNumber = 0; ///< Set number.
    std::unordered_map<uint32_t, DescriptorSetLayoutBinding> bindings = {}; ///< Descriptor set layout bindings
};

} // namespace chronicle
