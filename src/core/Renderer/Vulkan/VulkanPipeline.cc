// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanPipeline.h"

#include "Storage/Storage.h"

#include "VulkanEnums.h"
#include "VulkanInstance.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"

#include <spirv-reflect/spirv_reflect.h>

namespace chronicle {

CHR_CONCRETE(VulkanPipeline);

VulkanPipeline::VulkanPipeline(const PipelineInfo& pipelineInfo, const char* debugName)
    : _shader(pipelineInfo.shader)
    , _vertexBuffers(pipelineInfo.vertexBuffers)
{
    CHRZONE_RENDERER;

    assert(pipelineInfo.shader);
    assert(pipelineInfo.vertexBuffers.size() > 0);

    // CHRLOG_DEBUG("Create pipeline: shaders count={}, vertex buffer descriptions count={}",
    // pipelineInfo.shaders.size(),
    //     pipelineInfo.vertexBuffers.size());

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    if (debugName != nullptr)
        _debugName = debugName;
#endif // VULKAN_ENABLE_DEBUG_MARKER

    // descriptor sets layout
    _descriptorSetsLayout = getDescriptorSetsLayout();

    // create the pipeline
    create();

    // register the debug show lines event
    VulkanContext::dispatcher.sink<DebugShowLinesEvent>().connect<&VulkanPipeline::debugShowLines>(this);
}

VulkanPipeline::~VulkanPipeline()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy pipeline");

    // cleanup the pipeline
    cleanup();

    // destroy the descriptor sets layout
    for (const auto& descriptorSetLayout : _descriptorSetsLayout)
        VulkanContext::device.destroyDescriptorSetLayout(descriptorSetLayout);
}

PipelineRef VulkanPipeline::create(const PipelineInfo& pipelineInfo, const char* debugName)
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanPipeline>(pipelineInfo, debugName);
}

void VulkanPipeline::create()
{
    CHRZONE_RENDERER;

    const auto vulkanShader = static_cast<VulkanShader*>(_shader.get());

    // prepare the shaders
    const auto& stages = _shader->stages();
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {};
    shaderStages.reserve(stages.size());
    for (auto const& shaderStage : stages) {
        vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.setStage(VulkanEnums::shaderStageToVulkan(shaderStage));
        shaderStageCreateInfo.setModule(vulkanShader->shaderModule(shaderStage));
        shaderStageCreateInfo.setPName(vulkanShader->entryPoint(shaderStage).c_str());
        shaderStages.push_back(shaderStageCreateInfo);
    }

    // create binding and attribute descriptions for vertex input state
    auto attributeDescriptionsCount = 0;
    for (const auto& vertexBufferInfo : _vertexBuffers)
        attributeDescriptionsCount += vertexBufferInfo.attributeDescriptions.size();

    // reserve memory for the binding descripion vector
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {};
    bindingDescriptions.reserve(_vertexBuffers.size());

    // reserve memory for the input attribute descripion vector
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};
    attributeDescriptions.reserve(attributeDescriptionsCount);

    // fill the binding and attribute descriptions
    for (auto i = 0; i < _vertexBuffers.size(); i++) {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = i;
        bindingDescription.stride = _vertexBuffers[i].stride;
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        bindingDescriptions.push_back(bindingDescription);

        // fill the attribute descriptions
        for (auto j = 0; j < _vertexBuffers[i].attributeDescriptions.size(); j++) {
            vk::VertexInputAttributeDescription attributeDescription = {};
            attributeDescription.binding = i;
            attributeDescription.location = _vertexBuffers[i].attributeDescriptions[j].location;
            attributeDescription.format
                = VulkanEnums::formatToVulkan(_vertexBuffers[i].attributeDescriptions[j].format);
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
    vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
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

    // create the pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setSetLayouts(_descriptorSetsLayout);
    _pipelineLayout = VulkanContext::device.createPipelineLayout(pipelineLayoutInfo);

    // graphics pipeline
    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.setStages(shaderStages);
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

    // create the graphics pipeline
    vk::Result result;
    std::tie(result, _graphicsPipeline) = VulkanContext::device.createGraphicsPipeline(nullptr, graphicsPipelineInfo);
    if (result != vk::Result::eSuccess)
        throw RendererError("Failed to create graphics pipeline");

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    // set the debug object name
    VulkanUtils::setDebugObjectName(_graphicsPipeline, _debugName.c_str());
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

void VulkanPipeline::cleanup()
{
    CHRZONE_RENDERER;

    // get garbage collector
    auto& garbageCollector = VulkanContext::framesData[VulkanContext::currentFrame].garbageCollector;

    // add data to destroy to the garbage collector
    garbageCollector.emplace_back(_graphicsPipeline);
    garbageCollector.emplace_back(_pipelineLayout);

    // descriptor sets clear
    _descriptorSets.clear();
}

void VulkanPipeline::debugShowLines([[maybe_unused]] const DebugShowLinesEvent& evn)
{
    //CHRLOG_DEBUG("Recreate pipeline: shaders count={}, vertex buffer descriptions count={}", _shaderStages.size(),
    //    _vertexBuffers.size());

    // after a debug show line event, cleanup and recreate the pipeline
    cleanup();
    create();
}

std::vector<vk::DescriptorSetLayout> VulkanPipeline::getDescriptorSetsLayout() const
{
    CHRZONE_RENDERER;

    const auto& shaderDescriptorSetLayouts = _shader->descriptorSetLayouts();

    // reserve descriptor sets layout memory
    std::vector<vk::DescriptorSetLayout> descriptorSetsLayout;
    descriptorSetsLayout.resize(shaderDescriptorSetLayouts.size());

    // create the descriptor set layouts
    for (uint32_t i = 0; i < shaderDescriptorSetLayouts.size(); i++) {
        const auto& shaderDescriptorSetLayout = shaderDescriptorSetLayouts[i];
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
        for (const auto& shaderBinding : shaderDescriptorSetLayout.bindings) {
            vk::DescriptorSetLayoutBinding binding = {};
            binding.setBinding(shaderBinding.binding);
            binding.setDescriptorType(VulkanEnums::descriptorTypeFromVulkan(shaderBinding.descriptorType));
            binding.setDescriptorCount(shaderBinding.descriptorCount);
            binding.setStageFlags(VulkanEnums::shaderStageFlagsToVulkan(shaderBinding.stageFlags));
            bindings.push_back(binding);
        }

        // create the descriptor set layout
        vk::DescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.setBindings(bindings);
        descriptorSetsLayout[i] = VulkanContext::device.createDescriptorSetLayout(createInfo);
    }

    // return descriptor set layouts
    return descriptorSetsLayout;
}

} // namespace chronicle