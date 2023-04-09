#pragma once

#include "Renderer/Common.h"
#include "Systems/Systems.h"

namespace chronicle {

struct Locator {
    static inline RendererUnique renderer = {};
    static inline SystemsUnique systems = {};
};
} // namespace chronicle