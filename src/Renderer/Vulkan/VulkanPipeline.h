#pragma once

#include "pch.h"

#include "Renderer/PipelineI.h"

namespace chronicle {

struct DescriptorSetLayoutData {
    uint32_t setNumber = 0;
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
};

class VulkanPipeline : public PipelineI<VulkanPipeline>, private NonCopyable<VulkanPipeline> {
protected:
    explicit VulkanPipeline(const vk::Device& device, const PipelineInfo& pipelineInfo);

public:
    ~VulkanPipeline();

    [[nodiscard]] const vk::Pipeline& pipeline() const { return _graphicsPipeline; }
    [[nodiscard]] const vk::PipelineLayout& pipelineLayout() const { return _pipelineLayout; }

    static PipelineRef create(const Renderer* renderer, const PipelineInfo& pipelineInfo);

private:
    vk::Device _device;

    std::vector<vk::DescriptorSetLayout> _descriptorSetsLayout;
    vk::PipelineLayout _pipelineLayout;
    vk::Pipeline _graphicsPipeline;

    vk::DescriptorPool _descriptorPool;
    std::vector<vk::DescriptorSet> _descriptorSets;

    vk::ShaderModule createShaderModule(const std::vector<char>& code) const;
    std::vector<vk::DescriptorSetLayout> getDescriptorSetsLayout(
        const std::vector<std::vector<char>>& shadersCode) const;
    std::vector<DescriptorSetLayoutData> getDescriptorSetsLayout(const std::vector<char>& code) const;
};

} // namespace chronicle