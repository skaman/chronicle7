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
    /// @param name Frame buffer name.
    explicit VulkanFrameBuffer(const FrameBufferInfo& frameBufferInfo, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanFrameBuffer();

    /// @brief @see FrameBufferI#frameBufferId
    [[nodiscard]] FrameBufferId frameBufferId() const { return _framebuffer; }

    /// @brief @see FrameBufferI#create
    [[nodiscard]] static FrameBufferRef create(const FrameBufferInfo& frameBufferInfo, const std::string& name);

private:
    std::string _name; ///< Name.
    vk::Framebuffer _framebuffer = nullptr; ///< Vulkan frame buffer.
};

} // namespace chronicle