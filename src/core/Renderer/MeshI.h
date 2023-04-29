// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "VertexBufferInfo.h"

namespace chronicle {

/// @brief Submesh data used to construct the mesh.
struct Submesh {
    /// @brief Vertices count.
    uint32_t verticesCount = 0;

    /// @brief Vertex buffers.
    std::vector<VertexBufferRef> vertexBuffers = {};

    /// @brief Vertex buffers informations.
    std::vector<VertexBufferInfo> vertexBuffersInfo = {};

    /// @brief Indices count.
    uint32_t indicesCount = 0;

    /// @brief Index type.
    IndexType indexType = IndexType::undefined;

    /// @brief Index buffer.
    IndexBufferRef indexBuffer = {};

    /// @brief Material.
    MaterialRef material = {};
};

/// @brief Object used to handle a mesh.
/// @tparam T Type with implementation.
template <class T> class MeshI {
public:
    /// @brief Get the count of the submeshes that compose the mesh.
    /// @return Submesh count.
    [[nodiscard]] uint32_t submeshCount() const { return static_cast<const T*>(this)->submeshCount(); }

    /// @brief Get the vertices count for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Vertices count.
    [[nodiscard]] uint32_t verticesCount(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->verticesCount(submeshIndex);
    }

    /// @brief Get the vertex buffers for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Vertex buffers.
    [[nodiscard]] std::vector<VertexBufferRef> vertexBuffers(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->vertexBuffers(submeshIndex);
    }

    /// @brief Get the vertex buffers informations for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Vertex buffers informations.
    [[nodiscard]] std::vector<VertexBufferInfo> vertexBuffersInfo(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->vertexBuffersInfo(submeshIndex);
    }

    /// @brief Get the indices count for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Indices count.
    [[nodiscard]] uint32_t indicesCount(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->indicesCount(submeshIndex);
    }

    /// @brief Get the index type for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Index type.
    [[nodiscard]] IndexType indexType(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->indexType(submeshIndex);
    }

    /// @brief Get the index buffer for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Index buffer.
    [[nodiscard]] IndexBufferRef indexBuffer(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->indexBuffer(submeshIndex);
    }

    /// @brief Get the material for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Material.
    [[nodiscard]] MaterialRef material(uint32_t submeshIndex = 0) const
    {
        return static_cast<const T*>(this)->material(submeshIndex);
    }

    /// @brief Factory for create a new mesh.
    /// @param submeshes Submeshes that compose the mesh.
    /// @return The mesh.
    [[nodiscard]] static MeshRef create(const std::vector<Submesh>& submeshes) { return T::create(submeshes); }

private:
    MeshI() = default;
    friend T;
};

} // namespace chronicle