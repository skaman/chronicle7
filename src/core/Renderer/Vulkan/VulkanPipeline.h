// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/PipelineI.h"
#include "VulkanCommon.h"
#include "VulkanEvents.h"

namespace chronicle {

/// @brief Data structure that contain descriptor set informations read from the shader itself with spirv-reflect.
struct DescriptorSetLayoutData {
    uint32_t setNumber = 0; ///< Set number.
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {}; ///< Descriptor set layout bindings
};

/// @brief Vulkan implementation for @ref PipelineI
class VulkanPipeline : public PipelineI<VulkanPipeline>, private NonCopyable<VulkanPipeline> {
protected:
    /// @brief Default constructor.
    /// @param pipelineInfo Informations used to create a new pipeline.
    /// @param name Pipeline name.
    explicit VulkanPipeline(const PipelineInfo& pipelineInfo, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanPipeline();

    /// @brief @see PipelineI#pipelineId
    [[nodiscard]] PipelineId pipelineId() const { return _graphicsPipeline; }

    /// @brief @see PipelineI#pipelineLayoutId
    [[nodiscard]] PipelineLayoutId pipelineLayoutId() const { return _pipelineLayout; }

    /// @brief @see PipelineI#create
    [[nodiscard]] static PipelineRef create(const PipelineInfo& pipelineInfo, const std::string& name);

private:
    std::string _name; ///< Name.
    ShaderRef _shader; ///< Shader.
    RenderPassRef _renderPass; ///< Render pass.

    std::vector<vk::DescriptorSetLayout> _descriptorSetsLayout; ///< Descriptor sets layout.
    vk::PipelineLayout _pipelineLayout; ///< Pipeline layout.
    vk::Pipeline _graphicsPipeline; ///< Graphics pipeline.

    vk::DescriptorPool _descriptorPool; ///< Descriptor pool.
    std::vector<vk::DescriptorSet> _descriptorSets; ///< Descriptor sets.

    std::vector<VertexBufferInfo> _vertexBuffers; ///< Vertex buffers.

    /// @brief Create the pipeline.
    void create();

    /// @brief Cleanup the pipeline.
    void cleanup();

    /// @brief Callback for @ref DebugShowLinesEvent.
    void debugShowLines(const DebugShowLinesEvent& evn);

    /// @brief Get the descriptor sets layout from shader.
    /// @param descriptorSetsLayouts Descriptor sets layout.
    /// @return Vulkan descriptor sets layout.
    std::vector<vk::DescriptorSetLayout> getVulkanDescriptorSetsLayout(
        const std::vector<DescriptorSetLayout>& descriptorSetsLayout) const;
};

} // namespace chronicle