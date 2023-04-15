// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

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

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const = default;

    static VertexBufferInfo bufferInfo()
    {
        using enum chronicle::Format;

        return { .stride = sizeof(Vertex),
            .attributeDescriptions = { { .format = R32G32B32Sfloat, .offset = offsetof(Vertex, pos) },
                { .format = R32G32B32Sfloat, .offset = offsetof(Vertex, color) },
                { .format = R32G32Sfloat, .offset = offsetof(Vertex, texCoord) } } };
    }
};

} // namespace chronicle

namespace std {
template <> struct hash<chronicle::Vertex> {
    size_t operator()(chronicle::Vertex const& vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
            ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
} // namespace std