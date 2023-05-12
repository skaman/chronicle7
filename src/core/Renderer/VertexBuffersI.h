// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to handle a multiple vertex buffers.
/// @tparam T Type with implementation.
template <class T> class VertexBuffersI {
public:
    /// @brief Factory for create a new group of vertex buffers.
    /// @param vertexBuffers Vertex buffers.
    /// @param offsets Vertex buffer offsets.
    /// @return The vertex buffers.
    [[nodiscard]] static VertexBuffersRef create(
        const std::vector<VertexBufferRef>& vertexBuffers, const std::vector<uint32_t>& offsets)
    {
        return T::create(vertexBuffers, offsets);
    }

    /// @brief Factory for create a new group of vertex buffers.
    /// @param vertexBuffer Vertex buffer.
    /// @param offsetsVertex buffer offset.
    /// @return The vertex buffers.
    [[nodiscard]] static VertexBuffersRef create(const VertexBufferRef& vertexBuffer, uint32_t offset)
    {
        return T::create(vertexBuffer, offset);
    }

private:
    VertexBuffersI() = default;
    friend T;
};

} // namespace chronicle