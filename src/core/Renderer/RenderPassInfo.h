// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

struct RenderPassAttachment {
    Format format = Format::undefined;
    MSAA msaa = MSAA::sampleCount1;
    AttachmentLoadOp loadOp = AttachmentLoadOp::dontCare;
    AttachmentStoreOp storeOp = AttachmentStoreOp::dontCare;
    AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::dontCare;
    AttachmentStoreOp stencilStoreOp = AttachmentStoreOp::dontCare;
    ImageLayout initialLayout = ImageLayout::undefined;
    ImageLayout finalLayout = ImageLayout::undefined;
};

/// @brief Informations used to create a render pass.
struct RenderPassInfo {
    std::optional<RenderPassAttachment> colorAttachment = {};
    std::optional<RenderPassAttachment> depthStencilAttachment = {};
    std::optional<RenderPassAttachment> resolveAttachment = {};
};

} // namespace chronicle