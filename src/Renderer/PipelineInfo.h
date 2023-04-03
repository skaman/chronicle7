#pragma once

#include "pch.h"

#include "Common.h"
#include "DescriptorSet.h"
#include "RenderPass.h"
#include "VertexBufferInfo.h"

namespace chronicle {

struct PipelineInfo {
    std::shared_ptr<RenderPass> renderPass = {};
    std::unordered_map<ShaderStage, std::vector<char>> shaders = {};
    std::vector<VertexBufferInfo> vertexBuffers = {};
    std::vector<std::shared_ptr<DescriptorSet>> descriptorSets = {};
};

} // namespace chronicle