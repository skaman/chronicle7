// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

#include "PipelineInfo.h"
#include "TextureInfo.h"

namespace chronicle {

class App;

/// @brief GPU renderer.
/// @tparam T Type with implementation.
template <class T> class RendererI {
public:
    /// @brief Initialized the renderer.
    ///        This must be called after the platform initialization.
    static void init() { T::init(); }

    /// @brief Deinitialize the renderer.
    static void deinit() { T::deinit(); }

    /// @brief Wait for the GPU idle (all operations and frame in flights are completed)
    static void waitIdle() { T::waitIdle(); }

    /// @brief Invalidate the swapchain and recreate it.
    ///        It's called automatically by the platform.
    static void invalidateSwapChain() { T::invalidateSwapChain(); }

    /// @brief Begin a new frame.
    /// @return True if a new image is acquire. If false the loop tick should be skipped and try again.
    static bool beginFrame() { return T::beginFrame(); }

    /// @brief End a frame and submit data to the GPU.
    static void endFrame() { T::endFrame(); }

    /// @brief Get the activation status for the debug show lines tool.
    /// @return Activation status.
    [[nodiscard]] static bool debugShowLines() { return T::debugShowLines(); }

    /// @brief Set the activation status for the debug show lines tool.
    /// @param enabled Activation status.
    static void setDebugShowLines(bool enabled) { T::setDebugShowLines(enabled); }

    /// @brief Get the descriptor set for a specific frame.
    /// @return The descriptor set.
    [[nodiscard]] static const DescriptorSetRef& descriptorSet(uint32_t index) { return T::descriptorSet(index); }

    /// @brief Get the descriptor set for the current frame.
    /// @return The descriptor set.
    [[nodiscard]] static const DescriptorSetRef& descriptorSet() { return T::descriptorSet(); }

    /// @brief Get the command buffer for the current frame.
    /// @return The command buffer.
    [[nodiscard]] static const CommandBufferRef& commandBuffer() { return T::commandBuffer(); }

    /// @brief Get the swapchain width.
    /// @return Swapchain width.
    [[nodiscard]] static uint32_t width() { return T::width(); }

    /// @brief Get the swapchain height.
    /// @return Swapchain height.
    [[nodiscard]] static uint32_t height() { return T::height(); }

    /// @brief Get the number of max frames in flight.
    /// @return Max frames in flight.
    [[nodiscard]] static uint32_t maxFramesInFlight() { return T::maxFramesInFlight(); }

private:
    RendererI() = default;
    friend T;
};

} // namespace chronicle