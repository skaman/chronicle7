// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanPipeline.h"

#include "Storage/File.h"

#include "VulkanInstance.h"

#include <spirv-reflect/spirv_reflect.h>

namespace chronicle {

const int MAX_DESCRIPTOR_SETS = 4;

CHR_CONCRETE(VulkanPipeline);

VulkanPipeline::VulkanPipeline(const PipelineInfo& pipelineInfo)
    : _vertexBuffers(pipelineInfo.vertexBuffers)
{
    CHRZONE_RENDERER;

    assert(pipelineInfo.shaders.size() > 0);
    assert(pipelineInfo.vertexBuffers.size() > 0);

    CHRLOG_DEBUG("Create pipeline: shaders count={}, vertex buffer descriptions count={}", pipelineInfo.shaders.size(),
        pipelineInfo.vertexBuffers.size());

    _shaderStages.reserve(pipelineInfo.shaders.size());
    _shaderModules.reserve(pipelineInfo.shaders.size());

    std::vector<std::vector<char>> shadersCode = {};
    shadersCode.reserve(pipelineInfo.shaders.size());

    auto shaderIndex = 0;
    for (auto const& [shaderStage, filename] : pipelineInfo.shaders) {
        shadersCode.push_back(File::readBytes(filename));
        auto shaderModule = createShaderModule(shadersCode[shaderIndex]);
        vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.setStage(shaderStageToVulkan(shaderStage));
        shaderStageCreateInfo.setModule(shaderModule);
        shaderStageCreateInfo.setPName("main");

        _shaderStages.push_back(shaderStageCreateInfo);
        _shaderModules.push_back(shaderModule);

        shaderIndex++;
    }

    // descriptor sets layout
    _descriptorSetsLayout = getDescriptorSetsLayout(shadersCode);

    create();

    VulkanContext::dispatcher.sink<DebugShowLinesEvent>().connect<&VulkanPipeline::debugShowLines>(this);
}

VulkanPipeline::~VulkanPipeline()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy pipeline");

    cleanup();

    for (const auto& descriptorSetLayout : _descriptorSetsLayout)
        VulkanContext::device.destroyDescriptorSetLayout(descriptorSetLayout);

    for (auto const& shaderModule : _shaderModules)
        VulkanContext::device.destroyShaderModule(shaderModule);
}

PipelineRef VulkanPipeline::create(const PipelineInfo& pipelineInfo)
{
    return std::make_shared<ConcreteVulkanPipeline>(pipelineInfo);
}

void VulkanPipeline::create()
{
    CHRZONE_RENDERER;

    // create binding and attribute descriptions for vertex input state
    auto attributeDescriptionsCount = 0;
    for (const auto& vertexBufferInfo : _vertexBuffers)
        attributeDescriptionsCount += vertexBufferInfo.attributeDescriptions.size();

    std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {};
    bindingDescriptions.reserve(_vertexBuffers.size());

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};
    attributeDescriptions.reserve(attributeDescriptionsCount);

    for (auto i = 0; i < _vertexBuffers.size(); i++) {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = i;
        bindingDescription.stride = _vertexBuffers[i].stride;
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        bindingDescriptions.push_back(bindingDescription);

        for (auto j = 0; j < _vertexBuffers[i].attributeDescriptions.size(); j++) {
            vk::VertexInputAttributeDescription attributeDescription = {};
            attributeDescription.binding = i;
            attributeDescription.location = j;
            attributeDescription.format = formatToVulkan(_vertexBuffers[i].attributeDescriptions[j].format);
            attributeDescription.offset = _vertexBuffers[i].attributeDescriptions[j].offset;
            attributeDescriptions.push_back(attributeDescription);
        }
    }

    // vertex input state
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.setVertexBindingDescriptions(bindingDescriptions);
    vertexInputInfo.setVertexAttributeDescriptions(attributeDescriptions);

    // input assembly state
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
    inputAssembly.setPrimitiveRestartEnable(false);

    // view port state
    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.setViewportCount(1);
    viewportState.setScissorCount(1);

    // rasterizer state
    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.setDepthClampEnable(false);
    rasterizer.setRasterizerDiscardEnable(false);
    rasterizer.setPolygonMode(VulkanContext::debugShowLines ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);
    rasterizer.setDepthBiasEnable(false);

    // multisample state
    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.setSampleShadingEnable(false);
    multisampling.setRasterizationSamples(VulkanContext::msaaSamples);

    // depth stencil
    vk::PipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.setDepthTestEnable(true);
    depthStencil.setDepthWriteEnable(true);
    depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencil.setDepthBoundsTestEnable(false);
    depthStencil.setStencilTestEnable(false);

    // color blend attachment state
    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
        | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    colorBlendAttachment.setBlendEnable(false);

    // color blend state
    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.setLogicOpEnable(false);
    colorBlending.setLogicOp(vk::LogicOp::eCopy);
    colorBlending.setAttachments(colorBlendAttachment);
    colorBlending.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

    // dynamic state
    std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.setDynamicStates(dynamicStates);

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setSetLayouts(_descriptorSetsLayout);

    _pipelineLayout = VulkanContext::device.createPipelineLayout(pipelineLayoutInfo);

    // graphics pipeline
    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.setStages(_shaderStages);
    graphicsPipelineInfo.setPVertexInputState(&vertexInputInfo);
    graphicsPipelineInfo.setPInputAssemblyState(&inputAssembly);
    graphicsPipelineInfo.setPViewportState(&viewportState);
    graphicsPipelineInfo.setPRasterizationState(&rasterizer);
    graphicsPipelineInfo.setPMultisampleState(&multisampling);
    graphicsPipelineInfo.setPDepthStencilState(&depthStencil);
    graphicsPipelineInfo.setPColorBlendState(&colorBlending);
    graphicsPipelineInfo.setPDynamicState(&dynamicState);
    graphicsPipelineInfo.setLayout(_pipelineLayout);
    graphicsPipelineInfo.setRenderPass(VulkanContext::renderPass);
    graphicsPipelineInfo.setSubpass(0);

    vk::Result result;
    std::tie(result, _graphicsPipeline) = VulkanContext::device.createGraphicsPipeline(nullptr, graphicsPipelineInfo);
    if (result != vk::Result::eSuccess)
        throw RendererError("Failed to create graphics pipeline");
}

void VulkanPipeline::cleanup()
{
    CHRZONE_RENDERER;

    VulkanContext::device.destroyPipeline(_graphicsPipeline);
    VulkanContext::device.destroyPipelineLayout(_pipelineLayout);

    _descriptorSets.clear();
}

void VulkanPipeline::debugShowLines([[maybe_unused]] const DebugShowLinesEvent& evn)
{
    CHRLOG_DEBUG("Recreate pipeline: shaders count={}, vertex buffer descriptions count={}", _shaderStages.size(),
        _vertexBuffers.size());

    cleanup();
    create();
}

vk::ShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) const
{
    CHRZONE_RENDERER;

    return VulkanContext::device.createShaderModule(
        { vk::ShaderModuleCreateFlags(), code.size(), std::bit_cast<const uint32_t*>(code.data()) });
}

std::vector<vk::DescriptorSetLayout> VulkanPipeline::getDescriptorSetsLayout(
    const std::vector<std::vector<char>>& shadersCode) const
{
    CHRZONE_RENDERER;

    // set number -> binding id -> layout binding
    std::map<uint32_t, std::map<uint32_t, vk::DescriptorSetLayoutBinding>> sets = {};

    for (auto const& code : shadersCode) {
        auto descriptorSetsLayoutData = getDescriptorSetsLayout(code);
        for (const auto& descriptorSetLayoutData : descriptorSetsLayoutData) {
            // if set doesn't exist, create it
            if (!sets.contains(descriptorSetLayoutData.setNumber))
                sets[descriptorSetLayoutData.setNumber] = {};

            for (const auto& descriptorSetLayoutBinding : descriptorSetLayoutData.bindings) {
                // if binding doesn't exist, create it
                if (!sets[descriptorSetLayoutData.setNumber].contains(descriptorSetLayoutBinding.binding)) {
                    sets[descriptorSetLayoutData.setNumber][descriptorSetLayoutBinding.binding]
                        = descriptorSetLayoutBinding;
                } else {
                    sets[descriptorSetLayoutData.setNumber][descriptorSetLayoutBinding.binding].stageFlags
                        |= descriptorSetLayoutBinding.stageFlags;
                }
            }
        }
    }

    std::vector<vk::DescriptorSetLayout> descriptorSetsLayout;
    descriptorSetsLayout.resize(MAX_DESCRIPTOR_SETS);

    for (uint32_t i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {};

        if (sets.contains(i)) {
            bindings.reserve(sets[i].size());
            for (auto const& [bindingId, binding] : sets[i]) {
                bindings.push_back(binding);
            }
        }

        vk::DescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.setBindings(bindings);
        descriptorSetsLayout[i] = VulkanContext::device.createDescriptorSetLayout(createInfo);
    }

    return descriptorSetsLayout;
}

std::vector<DescriptorSetLayoutData> VulkanPipeline::getDescriptorSetsLayout(const std::vector<char>& code) const
{
    CHRZONE_RENDERER;

    SpvReflectShaderModule shaderModule = {};
    SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &shaderModule);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&shaderModule, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = spvReflectEnumerateDescriptorSets(&shaderModule, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<DescriptorSetLayoutData> layoutSets(sets.size(), DescriptorSetLayoutData {});
    for (size_t setIndex = 0; setIndex < sets.size(); ++setIndex) {
        const SpvReflectDescriptorSet& setRefl = *(sets[setIndex]);
        DescriptorSetLayoutData& layout = layoutSets[setIndex];
        layout.bindings.resize(setRefl.binding_count);
        for (uint32_t bindingIndex = 0; bindingIndex < setRefl.binding_count; ++bindingIndex) {
            const SpvReflectDescriptorBinding& bindingRefl = *(setRefl.bindings[bindingIndex]);
            vk::DescriptorSetLayoutBinding& layoutBinding = layout.bindings[bindingIndex];
            layoutBinding.setBinding(bindingRefl.binding);
            layoutBinding.descriptorType = static_cast<vk::DescriptorType>(bindingRefl.descriptor_type);
            layoutBinding.setDescriptorCount(1);
            for (uint32_t dimensionIndex = 0; dimensionIndex < bindingRefl.array.dims_count; ++dimensionIndex) {
                layoutBinding.setDescriptorCount(
                    layoutBinding.descriptorCount * bindingRefl.array.dims[dimensionIndex]);
            }
            layoutBinding.stageFlags = static_cast<vk::ShaderStageFlagBits>(shaderModule.shader_stage);
        }
        layout.setNumber = setRefl.set;
    }

    return layoutSets;
}

} // namespace chronicle