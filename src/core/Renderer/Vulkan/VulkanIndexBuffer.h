#pragma once

#include "pch.h"

#include "Renderer/IndexBufferI.h"

namespace chronicle {

class VulkanIndexBuffer : public IndexBufferI<VulkanIndexBuffer>, private NonCopyable<VulkanIndexBuffer> {
protected:
    explicit VulkanIndexBuffer() = default;

public:
    ~VulkanIndexBuffer();

    void set(void* src, size_t size);

    [[nodiscard]] const vk::Buffer& buffer() const { return _buffer; }

    static IndexBufferRef create();

private:
    vk::Buffer _buffer;
    vk::DeviceMemory _bufferMemory;

    void cleanup() const;
};

} // namespace chronicle