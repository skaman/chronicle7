#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

struct AttributeDescriptionInfo {
    Format format = Format::Undefined;
    uint32_t offset = 0;
};

struct VertexBufferInfo {
    uint32_t stride;
    std::vector<AttributeDescriptionInfo> attributeDescriptions;
};

} // namespace chronicle