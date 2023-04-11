#pragma once

#include "pch.h"

#include "Renderer/VertexBufferI.h"

namespace chronicle {

class VulkanVertexBuffer : public VertexBufferI<VulkanVertexBuffer>, private NonCopyable<VulkanVertexBuffer> {
protected:
    explicit VulkanVertexBuffer() = default;

public:
    ~VulkanVertexBuffer();

    void set(void* src, size_t size);

    [[nodiscard]] const vk::Buffer& buffer() const { return _buffer; }

    static VertexBufferRef create();

private:
    vk::Buffer _buffer;
    vk::DeviceMemory _bufferMemory;

    void cleanup() const;
};

} // namespace chronicle