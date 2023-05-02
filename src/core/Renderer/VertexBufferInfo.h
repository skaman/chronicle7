// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Type of the attribute.
enum class AttributeType { undefined, position, normal, textcoord0, color0, tangent, joints0, weights0 };

/// @brief Vertex buffer attribute descriptions.
struct AttributeDescriptionInfo {
    /// @brief Data format.
    Format format = Format::undefined;

    /// @brief Data offset into the structure.
    uint32_t offset = 0;

    /// @brief Attribute location.
    uint32_t location = 0;
};

/// @brief Information used to describe a vertext buffer layout.
struct VertexBufferInfo {
    /// @brief Data stride.
    uint32_t stride;

    /// @brief Attribute descriptions.
    std::vector<AttributeDescriptionInfo> attributeDescriptions;
};

} // namespace chronicle

template <> struct std::hash<chronicle::AttributeDescriptionInfo> {
    std::size_t operator()(const chronicle::AttributeDescriptionInfo& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.format, data.offset, data.location);
        return h;
    }
};

template <> struct std::hash<chronicle::VertexBufferInfo> {
    std::size_t operator()(const chronicle::VertexBufferInfo& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.stride);
        for (const auto& attributeDescription : data.attributeDescriptions) {
            std::hash_combine(h, attributeDescription);
        }
        return h;
    }
};