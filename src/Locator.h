#pragma once

#include "Renderer/Common.h"
//#include "Renderer/Vulkan/Vulkan.h"

namespace chronicle {

struct Locator {
    static inline RendererRef renderer = {};
};
} // namespace chronicle