// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Informations used to create a framebuffer.
struct FrameBufferInfo {
    /// @brief A vectore of texture handles, each of which will be used as the corresponding attachment in a render pass
    /// instance.
    std::vector<TextureId> attachments = {};

    /// @brief A render pass defining what render passes the framebuffer will be compatible with.
    RenderPassId renderPass = nullptr;

    /// @brief Framebuffer width.
    uint32_t width = 0;

    /// @brief Framebuffer height.
    uint32_t height = 0;
};

} // namespace chronicle