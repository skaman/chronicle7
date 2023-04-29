// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/MeshI.h"

namespace chronicle {

/// @brief Mesh binding informations.
struct VulkanMeshBindingInfo {
    std::vector<vk::Buffer> vertexBuffers; ///< Vulkan vertex buffers used for binding.
    std::vector<vk::DeviceSize> vertexBuffersOffsets; ///< Vulkan vertex buffers offset used for binding.
    vk::Buffer indexBuffer; ///< Vulkan index buffer used for binding.
    vk::DeviceSize indexBufferOffset; ///< Vulkan index buffer offset used for binding.
    vk::IndexType indexType; ///< Vulkan index type used for binding.
};

/// @brief Vulkan implementation for @ref MeshI
class VulkanMesh : public MeshI<VulkanMesh>, private NonCopyable<VulkanMesh> {
protected:
    /// @brief Default constructor.
    /// @param submeshes Submeshes that compose the mesh.
    explicit VulkanMesh(const std::vector<Submesh>& submeshes);

public:
    /// @brief Destructor.
    ~VulkanMesh() = default;

    /// @brief @see MeshI#submeshCount
    [[nodiscard]] uint32_t submeshCount() const { return static_cast<uint32_t>(_submeshes.size()); }

    /// @brief @see MeshI#verticesCount
    [[nodiscard]] uint32_t verticesCount(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].verticesCount;
    }

    /// @brief @see MeshI#vertexBuffers
    [[nodiscard]] std::vector<VertexBufferRef> vertexBuffers(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].vertexBuffers;
    }

    /// @brief @see MeshI#vertexBuffersInfo
    [[nodiscard]] std::vector<VertexBufferInfo> vertexBuffersInfo(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].vertexBuffersInfo;
    }

    /// @brief @see MeshI#indicesCount
    [[nodiscard]] uint32_t indicesCount(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].indicesCount;
    }

    /// @brief @see MeshI#indexType
    [[nodiscard]] IndexType indexType(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].indexType;
    }

    /// @brief @see MeshI#indexBuffer
    [[nodiscard]] IndexBufferRef indexBuffer(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].indexBuffer;
    }

    /// @brief @see MeshI#material
    [[nodiscard]] MaterialRef material(uint32_t submeshIndex) const
    {
        assert(_submeshes.size() > submeshIndex);
        return _submeshes[submeshIndex].material;
    }

    /// @brief @see MeshI#create
    [[nodiscard]] static MeshRef create(const std::vector<Submesh>& submeshes);

    /// @brief Get the binding informations.
    /// @param submeshIndex Submesh index.
    /// @return Mesh binding informations.
    [[nodiscard]] const VulkanMeshBindingInfo& bindingInfo(uint32_t submeshIndex) const
    {
        assert(_bindingInfos.size() > submeshIndex);
        return _bindingInfos[submeshIndex];
    }

private:
    std::vector<Submesh> _submeshes = {}; ///< Submeshes that compose the mesh.
    std::vector<VulkanMeshBindingInfo> _bindingInfos = {}; ///< Vulkan mesh binding informations.
};

} // namespace chronicle
