#pragma once

#include "pch.h"

#include "Renderer/PipelineInfo.h"

namespace chronicle {

class VulkanPipeline {
public:
    explicit VulkanPipeline(const vk::Device& device, const PipelineInfo& pipelineInfo);
    ~VulkanPipeline();

    // internal
    [[nodiscard]] inline const vk::Pipeline& pipeline() const { return _graphicsPipeline; }
    [[nodiscard]] inline const vk::PipelineLayout& pipelineLayout() const { return _pipelineLayout; }

private:
    vk::Device _device;

    vk::DescriptorSetLayout _descriptorSetLayout;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;

    vk::DescriptorPool _descriptorPool;
    std::vector<vk::DescriptorSet> _descriptorSets;

    vk::ShaderModule createShaderModule(const std::vector<char>& code) const;
};

} // namespace chronicle