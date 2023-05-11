// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "VertexBufferInfo.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanShader.h"
#endif

namespace chronicle {

/// @brief Informations used to create a new pipeline.
struct PipelineInfo {
    /// @brief Shader to be attached to the pipeline.
    ShaderRef shader = {};

    /// @brief Informations about the layout of the vertex buffers will be attached to the pipeline.
    std::vector<VertexBufferInfo> vertexBuffers = {};

    /// @brief Informations about the descriptor sets that will be attached to the pipeline.
    std::vector<DescriptorSetLayout> descriptorSetsLayout = {};
};

} // namespace chronicle

template <> struct std::hash<chronicle::PipelineInfo> {
    std::size_t operator()(const chronicle::PipelineInfo& data) const noexcept
    {
        std::size_t h = data.shader->hash();
        for (const auto& vertexBuffer : data.vertexBuffers) {
            std::hash_combine(h, vertexBuffer);
        }
        for (const auto& descriptorSetLayout : data.descriptorSetsLayout) {
            std::hash_combine(h, descriptorSetLayout);
        }
        return h;
    }
};
