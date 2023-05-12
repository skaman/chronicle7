// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Material.h"
#include "Renderer/Renderer.h"

namespace chronicle {

/// @brief Bounding box.
struct BoundingBox {
    /// @brief Bounding box minimal value.
    glm::vec3 min;

    /// @brief Bounding box maximun value.
    glm::vec3 max;
};

/// @brief Submesh data used to construct the mesh.
struct Submesh {
    /// @brief Vertices count.
    uint32_t verticesCount = 0;

    /// @brief Vertex buffers.
    VertexBuffersRef vertexBuffers = {};

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

    /// @brief Pipeline.
    PipelineRef pipeline = {};

    /// @brief Mesh bounding box.
    BoundingBox boundingBox = {};
};

class Mesh;
using MeshRef = std::shared_ptr<Mesh>;

/// @brief Object used to handle a mesh.
class Mesh : private NonCopyable<Mesh> {
protected:
    /// @brief Default constructor.
    /// @param submeshes Submeshes that compose the mesh.
    explicit Mesh(const std::vector<Submesh>& submeshes);

public:
    /// @brief Destructor.
    ~Mesh() = default;

    /// @brief Get the count of the submeshes that compose the mesh.
    /// @return Submesh count.
    [[nodiscard]] uint32_t submeshCount() const { return static_cast<uint32_t>(_submeshes.size()); }

    /// @brief Get the vertices count for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Vertices count.
    [[nodiscard]] uint32_t verticesCount(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].verticesCount;
    }

    /// @brief Get the vertex buffers for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Vertex buffers.
    [[nodiscard]] VertexBuffersRef vertexBuffers(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].vertexBuffers;
    }

    /// @brief Get the vertex buffers informations for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Vertex buffers informations.
    [[nodiscard]] std::vector<VertexBufferInfo> vertexBuffersInfo(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].vertexBuffersInfo;
    }

    /// @brief Get the indices count for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Indices count.
    [[nodiscard]] uint32_t indicesCount(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].indicesCount;
    }

    /// @brief Get the index type for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Index type.
    [[nodiscard]] IndexType indexType(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].indexType;
    }

    /// @brief Get the index buffer for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return Index buffer.
    [[nodiscard]] IndexBufferRef indexBuffer(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].indexBuffer;
    }

    /// @brief Get the material for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return The material.
    [[nodiscard]] MaterialRef material(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].material;
    }

    /// @brief Get the pipeline for a specific submesh.
    /// @param submeshIndex Submesh index.
    /// @return The pipeline.
    [[nodiscard]] PipelineRef pipeline(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].pipeline;
    }

    /// @brief Factory for create a new mesh.
    /// @param submeshes Submeshes that compose the mesh.
    /// @return The mesh.
    [[nodiscard]] static MeshRef create(const std::vector<Submesh>& submeshes);

private:
    std::vector<Submesh> _submeshes = {}; ///< Submeshes that compose the mesh.
};

} // namespace chronicle
