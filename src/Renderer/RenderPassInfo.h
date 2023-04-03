#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

class Image;

struct RenderPassInfo {
    Format colorAttachmentFormat = Format::Undefined;
    std::vector<std::shared_ptr<Image>> images = {};
};

} // namespace chronicle