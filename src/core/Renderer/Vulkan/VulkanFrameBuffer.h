// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseFrameBuffer.h"

namespace chronicle::internal::vulkan {

/// @brief Vulkan implementation for @ref FrameBufferI
class VulkanFrameBuffer : public BaseFrameBuffer<VulkanFrameBuffer>, private NonCopyable<VulkanFrameBuffer> {
protected:
    /// @brief Constructor.
    /// @param frameBufferInfo Informations used to create the frame buffer.
    /// @param name Frame buffer name.
    explicit VulkanFrameBuffer(const FrameBufferInfo& frameBufferInfo, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanFrameBuffer();

    /// @brief @see BaseFrameBuffer#frameBufferId
    [[nodiscard]] FrameBufferId frameBufferId() const { return _framebuffer; }

    /// @brief @see BaseFrameBuffer#create
    [[nodiscard]] static FrameBufferRef create(const FrameBufferInfo& frameBufferInfo, const std::string& name);

private:
    std::string _name {}; ///< Name.
    vk::Framebuffer _framebuffer {}; ///< Vulkan frame buffer.
};

} // namespace chronicle