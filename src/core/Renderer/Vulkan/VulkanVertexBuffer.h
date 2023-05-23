// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseVertexBuffer.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref BaseVertexBuffer
class VulkanVertexBuffer : public BaseVertexBuffer<VulkanVertexBuffer>, private NonCopyable<VulkanVertexBuffer> {
protected:
    /// @brief Default constructor.
    explicit VulkanVertexBuffer(const uint8_t* src, size_t size, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanVertexBuffer();

    /// @brief @see IndexBufferI#vertexBufferId
    [[nodiscard]] VertexBufferId vertexBufferId() const { return _buffer; }

    /// @brief @see BaseVertexBuffer#create
    [[nodiscard]] static VertexBufferRef create(const std::vector<uint8_t>& data, const std::string& name);

    /// @brief @see BaseVertexBuffer#create
    [[nodiscard]] static VertexBufferRef create(const uint8_t* src, size_t size, const std::string& name);

private:
    std::string _name {}; ///< Name.
    vk::Buffer _buffer {}; ///< Buffer.
    vk::DeviceMemory _bufferMemory {}; ///< Device memory for the buffer.
};

} // namespace chronicle