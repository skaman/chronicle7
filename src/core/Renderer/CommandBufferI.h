// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to record command which can be sebsequently submitted to GPU for execution.
/// @tparam T Type with implementation.
template <class T> class CommandBufferI {
public:
    /// @brief Draw primitives with indexed vertices.
    /// @param indexCount The number of vertices to draw.
    /// @param instanceCount The number of instances to draw.
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
    {
        static_cast<const T*>(this)->drawIndexed(indexCount, instanceCount);
    }

    /// @brief Bind a pipeline object to the command buffer.
    /// @param pipeline The pipeline to be bound.
    void bindPipeline(const PipelineRef& pipeline) { static_cast<T*>(this)->bindPipeline(pipeline); }

    /// @brief Bind a vertex buffer to the command buffer.
    /// @param vertexBuffer The vertex buffer to be bound.
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
    {
        static_cast<const T*>(this)->bindVertexBuffer(vertexBuffer);
    }

    /// @brief Bind an index buffer to the command buffer.
    /// @param indexBuffer The index buffer to be bound.
    void bindIndexBuffer(const IndexBufferRef& indexBuffer) const
    {
        static_cast<const T*>(this)->bindIndexBuffer(indexBuffer);
    }

    /// @brief Bind a mesh to the command buffer.
    /// @param mesh The mesh to be bound.
    /// @param submeshIndex The submesh index.
    void bindMesh(const MeshRef& mesh, uint32_t submeshIndex = 0) const
    {
        static_cast<const T*>(this)->bindMesh(mesh, submeshIndex);
    }

    /// @brief Bind a descriptor set to the command buffer.
    /// @param descriptorSet The descriptor set to be bound.
    /// @param index The number of the descriptor to be bound.
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSet(descriptorSet, index);
    }

    /// @brief Begin a debug label.
    /// @param name Label name.
    /// @param color Label color.
    void beginDebugLabel(const char* name, glm::vec4 color) const
    {
        static_cast<const T*>(this)->beginDebugLabel(name, color);
    }

    /// @brief End a debug label.
    void endDebugLabel() const { static_cast<const T*>(this)->endDebugLabel(); }

    /// @brief Insert a debug label.
    /// @param name Label name.
    /// @param color Label color.
    void insertDebugLabel(const char* name, glm::vec4 color) const
    {
        static_cast<const T*>(this)->insertDebugLabel(name, color);
    }

private:
    CommandBufferI() = default;
    friend T;
};

} // namespace chronicle