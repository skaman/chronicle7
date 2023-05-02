// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/VertexBuffersI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref VertexBufferI
class VulkanVertexBuffers : public VertexBuffersI<VulkanVertexBuffers>, private NonCopyable<VulkanVertexBuffers> {
protected:
    /// @brief Default constructor.
    explicit VulkanVertexBuffers(
        const std::vector<VertexBufferRef>& vertexBuffers, const std::vector<uint32_t>& offsets);

public:
    /// @brief Destructor.
    ~VulkanVertexBuffers() = default;

    /// @brief Get the vulkan handles for the buffers.
    /// @return Vulkan handle.
    [[nodiscard]] const std::vector<vk::Buffer>& buffers() const { return _buffers; }

    /// @brief Get the offsets related to the buffers.
    /// @return Offsets.
    [[nodiscard]] const std::vector<vk::DeviceSize>& offsets() const { return _offsets; }

    /// @brief @see VertexBuffersI#create
    [[nodiscard]] static VertexBuffersRef create(
        const std::vector<VertexBufferRef>& vertexBuffers, const std::vector<uint32_t>& offsets);

private:
    std::vector<VertexBufferRef> _vertexBuffers = {}; ///< Vertex buffers.
    std::vector<vk::Buffer> _buffers = {}; ///< Vulkan buffers.
    std::vector<vk::DeviceSize> _offsets = {}; ///< Vertex buffer offsets.
};

} // namespace chronicle