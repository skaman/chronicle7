// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/IndexBufferI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref IndexBufferI
class VulkanIndexBuffer : public IndexBufferI<VulkanIndexBuffer>, private NonCopyable<VulkanIndexBuffer> {
protected:
    /// @brief Default constructor.
    explicit VulkanIndexBuffer() = default;

public:
    /// @brief Destructor.
    ~VulkanIndexBuffer();

    /// @brief @see IndexBufferI#set
    void set(void* src, size_t size);

    /// @brief Get the vulkan handle for the buffer.
    /// @return Vulkan handle.
    [[nodiscard]] const vk::Buffer& buffer() const { return _buffer; }

    /// @brief @see IndexBufferI#create
    [[nodiscard]] static IndexBufferRef create();

private:
    vk::Buffer _buffer; ///< Buffer.
    vk::DeviceMemory _bufferMemory; ///< Device memory for the buffer.

    /// @brief Cleanup resources.
    void cleanup() const;
};

} // namespace chronicle