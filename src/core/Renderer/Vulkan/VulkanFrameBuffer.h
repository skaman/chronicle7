// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/FrameBufferI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref FrameBufferI
class VulkanFrameBuffer : public FrameBufferI<VulkanFrameBuffer>, private NonCopyable<VulkanFrameBuffer> {
protected:
    /// @brief Constructor.
    /// @param frameBufferInfo Informations used to create the frame buffer.
    explicit VulkanFrameBuffer(const FrameBufferInfo& frameBufferInfo);

public:
    /// @brief Destructor.
    ~VulkanFrameBuffer();

    /// @brief @see FrameBufferI#frameBufferId
    [[nodiscard]] FrameBufferId frameBufferId() const { return static_cast<FrameBufferId>(&_framebuffer); }

    /// @brief @see FrameBufferI#create
    [[nodiscard]] static FrameBufferRef create(const FrameBufferInfo& frameBufferInfo);

private:
    vk::Framebuffer _framebuffer = nullptr; ///< Vulkan frame buffer.
};

} // namespace chronicle