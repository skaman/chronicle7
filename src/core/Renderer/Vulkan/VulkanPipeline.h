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
    explicit VulkanPipeline(const PipelineInfo& pipelineInfo);

public:
    /// @brief Destructor.
    ~VulkanPipeline();

    /// @brief Get the vulkan handle for pipeline.
    /// @return Vulkan handle.
    [[nodiscard]] const vk::Pipeline& pipeline() const { return _graphicsPipeline; }

    /// @brief Get the pipeline layout.
    /// @return Pipeline layout.
    [[nodiscard]] const vk::PipelineLayout& pipelineLayout() const { return _pipelineLayout; }

    /// @brief @see PipelineI#create
    [[nodiscard]] static PipelineRef create(const PipelineInfo& pipelineInfo);

private:
    std::vector<vk::DescriptorSetLayout> _descriptorSetsLayout; ///< Descriptor sets layout.
    vk::PipelineLayout _pipelineLayout; ///< Pipeline layout.
    vk::Pipeline _graphicsPipeline; ///< Graphics pipeline.

    vk::DescriptorPool _descriptorPool; ///< Descriptor pool.
    std::vector<vk::DescriptorSet> _descriptorSets; ///< Descriptor sets.

    std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages; ///< Shader stages create informations.
    std::vector<vk::ShaderModule> _shaderModules; ///< Shader modules.
    std::vector<VertexBufferInfo> _vertexBuffers; ///< Vertex buffers.

    /// @brief Create the pipeline.
    void create();

    /// @brief Cleanup the pipeline.
    void cleanup();

    /// @brief Callback for @ref DebugShowLinesEvent.
    void debugShowLines(const DebugShowLinesEvent& evn);

    /// @brief Create the shader module from spir-v code.
    /// @param code Spir-v code.
    /// @return Shader module.
    vk::ShaderModule createShaderModule(const std::vector<char>& code) const;

    /// @brief Get the descriptor sets layout from spir-v code of multiple shaders.
    /// @param shadersCode A vector of shaders spir-v code.
    /// @return Descriptor sets layout.
    std::vector<vk::DescriptorSetLayout> getDescriptorSetsLayout(
        const std::vector<std::vector<char>>& shadersCode) const;

    /// @brief Get the descriptor sets layout data from spir-v code of a single shader.
    /// @param code Shaders spir-v code.
    /// @return Descriptor sets layout data.
    std::vector<DescriptorSetLayoutData> getDescriptorSetsLayout(const std::vector<char>& code) const;
};

} // namespace chronicle