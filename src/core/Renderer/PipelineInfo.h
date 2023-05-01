// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "VertexBufferInfo.h"

namespace chronicle {

/// @brief Informations used to create a new pipeline.
struct PipelineInfo {
    /// @brief Shader to be attached to the pipeline.
    ShaderRef shader = {};

    /// @brief Informations about the layout of the vertex buffers will be attached to the pipeline.
    std::vector<VertexBufferInfo> vertexBuffers = {};
};

} // namespace chronicle