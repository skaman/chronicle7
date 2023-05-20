// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

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
    RenderPassAttachment colorAttachment = {};
    std::optional<RenderPassAttachment> depthStencilAttachment = {};
    std::optional<RenderPassAttachment> resolveAttachment = {};
};

} // namespace chronicle

template <> struct std::hash<chronicle::RenderPassAttachment> {
    std::size_t operator()(const chronicle::RenderPassAttachment& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.format, data.msaa, data.loadOp, data.storeOp, data.stencilLoadOp, data.stencilStoreOp,
            data.initialLayout, data.finalLayout);
        return h;
    }
};

template <> struct std::hash<chronicle::RenderPassInfo> {
    std::size_t operator()(const chronicle::RenderPassInfo& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.colorAttachment, data.depthStencilAttachment, data.resolveAttachment);
        return h;
    }
};
