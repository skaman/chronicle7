// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

#include "PipelineInfo.h"
#include "TextureInfo.h"

namespace chronicle {

class App;

template <class T> class RendererI {
public:
    static void init() { T::init(); }
    static void deinit() { T::deinit(); }

    static void waitIdle() { T::waitIdle(); }
    static void invalidateSwapChain() { T::invalidateSwapChain(); }

    static bool beginFrame() { return T::beginFrame(); }
    static void endFrame() { T::endFrame(); }

    [[nodiscard]] static bool debugShowLines() { return T::debugShowLines(); }
    static void setDebugShowLines(bool enabled) { T::setDebugShowLines(enabled); }

    [[nodiscard]] static const std::vector<DescriptorSetRef>& descriptorSets() { return T::descriptorSets(); }
    [[nodiscard]] static const DescriptorSetRef& descriptorSet() { return T::descriptorSet(); }
    [[nodiscard]] static const CommandBufferRef& commandBuffer() { return T::commandBuffer(); }
    [[nodiscard]] static uint32_t width() { return T::width(); }
    [[nodiscard]] static uint32_t height() { return T::height(); }

private:
    RendererI() = default;
    friend T;
};

} // namespace chronicle