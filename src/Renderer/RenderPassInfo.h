#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

struct RenderPassInfo {
    Format colorAttachmentFormat = Format::Undefined;
    Format depthAttachmentFormat = Format::Undefined;
    std::vector<ImageRef> images = {};
    ImageRef depthImage = {};
};

} // namespace chronicle