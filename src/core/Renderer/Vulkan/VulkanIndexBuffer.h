// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseIndexBuffer.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref BaseIndexBuffer
class VulkanIndexBuffer : public BaseIndexBuffer<VulkanIndexBuffer>, private NonCopyable<VulkanIndexBuffer> {
protected:
    /// @brief Default constructor.
    explicit VulkanIndexBuffer(const uint8_t* src, size_t size, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanIndexBuffer();

    /// @brief @see BaseIndexBuffer#indexBufferId
    [[nodiscard]] IndexBufferId indexBufferId() const { return _buffer; }

    /// @brief @see BaseIndexBuffer#create
    [[nodiscard]] static IndexBufferRef create(const std::vector<uint8_t>& data, const std::string& name);

    /// @brief @see BaseIndexBuffer#create
    [[nodiscard]] static IndexBufferRef create(const uint8_t* src, size_t size, const std::string& name);

private:
    std::string _name {}; ///< Name.
    vk::Buffer _buffer {}; ///< Buffer.
    vk::DeviceMemory _bufferMemory {}; ///< Device memory for the buffer.
};

} // namespace chronicle