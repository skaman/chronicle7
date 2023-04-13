#pragma once

#include "pch.h"

#include "Renderer/Common.h"

namespace chronicle {

class Gui {
public:
    static void init(uint32_t swapChainImagesCount, const RenderPassRef& renderPass);
    static void deinit();

    static void newFrame();
    static void render(const CommandBufferRef& commandBuffer);
};

} // namespace chronicle