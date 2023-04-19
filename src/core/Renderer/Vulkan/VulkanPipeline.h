// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/PipelineI.h"
#include "VulkanCommon.h"

namespace chronicle {

struct DescriptorSetLayoutData {
    uint32_t setNumber = 0;
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
};

class VulkanPipeline : public PipelineI<VulkanPipeline>, private NonCopyable<VulkanPipeline> {
protected:
    explicit VulkanPipeline(const PipelineInfo& pipelineInfo);

public:
    ~VulkanPipeline();

    [[nodiscard]] const vk::Pipeline& pipeline() const { return _graphicsPipeline; }
    [[nodiscard]] const vk::PipelineLayout& pipelineLayout() const { return _pipelineLayout; }

    [[nodiscard]] static PipelineRef create(const PipelineInfo& pipelineInfo);

private:
    std::vector<vk::DescriptorSetLayout> _descriptorSetsLayout;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;

    vk::DescriptorPool _descriptorPool;
    std::vector<vk::DescriptorSet> _descriptorSets;

    std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages;
    std::vector<vk::ShaderModule> _shaderModules;
    std::vector<VertexBufferInfo> _vertexBuffers;

    void create();
    void cleanup();

    void debugShowLines(const DebugShowLinesEvent& evn);

    vk::ShaderModule createShaderModule(const std::vector<char>& code) const;
    std::vector<vk::DescriptorSetLayout> getDescriptorSetsLayout(
        const std::vector<std::vector<char>>& shadersCode) const;
    std::vector<DescriptorSetLayoutData> getDescriptorSetsLayout(const std::vector<char>& code) const;
};

} // namespace chronicle