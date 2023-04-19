// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Vertex buffer attribute descriptions.
struct AttributeDescriptionInfo {
    /// @brief Data format.
    Format format = Format::Undefined;

    /// @brief Data offset into the structure.
    uint32_t offset = 0;
};

/// @brief Information used to describe a vertext buffer layout.
struct VertexBufferInfo {
    /// @brief Data stride.
    uint32_t stride;

    /// @brief Attribute descriptions.
    std::vector<AttributeDescriptionInfo> attributeDescriptions;
};

} // namespace chronicle