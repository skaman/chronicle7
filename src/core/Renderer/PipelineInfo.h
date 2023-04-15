// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "VertexBufferInfo.h"

namespace chronicle {

struct PipelineInfo {
    RenderPassRef renderPass = {};
    std::unordered_map<ShaderStage, std::string> shaders = {};
    std::vector<VertexBufferInfo> vertexBuffers = {};
};

} // namespace chronicle