// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/VertexBufferI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref VertexBufferI
class VulkanVertexBuffer : public VertexBufferI<VulkanVertexBuffer>, private NonCopyable<VulkanVertexBuffer> {
protected:
    /// @brief Default constructor.
    explicit VulkanVertexBuffer() = default;

public:
    /// @brief Destructor.
    ~VulkanVertexBuffer();

    /// @brief @see VertexBufferI#set
    void set(void* src, size_t size, const char* debugName);

    /// @brief Get the vulkan handle for the buffer.
    /// @return Vulkan handle.
    [[nodiscard]] const vk::Buffer& buffer() const { return _buffer; }

    /// @brief @see VertexBufferI#create
    [[nodiscard]] static VertexBufferRef create();

private:
    vk::Buffer _buffer; ///< Buffer.
    vk::DeviceMemory _bufferMemory; ///< Device memory for the buffer.

    /// @brief Cleanup resources.
    void cleanup() const;
};

} // namespace chronicle