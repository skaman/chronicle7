#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

struct RenderPassInfo {
    Format colorAttachmentFormat = Format::Undefined;
    std::vector<ImageRef> images = {};
};

} // namespace chronicle