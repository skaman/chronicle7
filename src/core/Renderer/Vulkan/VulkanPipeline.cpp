// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanPipeline.h"

#include "Storage/StorageContext.h"

#include "VulkanEnums.h"
#include "VulkanGC.h"
#include "VulkanInstance.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"

namespace chronicle::internal::vulkan {

CHR_CONCRETE(VulkanPipeline);

VulkanPipeline::VulkanPipeline(const PipelineInfo& pipelineInfo, const std::string& name)
    : _name(name)
    , _shader(pipelineInfo.shader)
    , _renderPass(pipelineInfo.renderPass)
    , _vertexBuffers(pipelineInfo.vertexBuffers)
{
    CHRZONE_RENDERER;

    assert(_shader);
    assert(_vertexBuffers.size() > 0);

    // descriptor sets layout
    _descriptorSetsLayout = getVulkanDescriptorSetsLayout(pipelineInfo.descriptorSetsLayout);

    // create the pipeline
    create();

    // register the debug show lines event
    VulkanContext::dispatcher.sink<DebugShowLinesEvent>().connect<&VulkanPipeline::debugShowLines>(this);
}

VulkanPipeline::~VulkanPipeline()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("Destroy pipeline");

    // cleanup the pipeline
    cleanup();

    // destroy the descriptor sets layout
    for (const auto& descriptorSetLayout : _descriptorSetsLayout)
        VulkanContext::device.destroyDescriptorSetLayout(descriptorSetLayout);
}

PipelineRef VulkanPipeline::create(const PipelineInfo& pipelineInfo, const std::string& name)
{
    // create an instance of the class
    return std::make_shared<ConcreteVulkanPipeline>(pipelineInfo, name);
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
        for (const auto& sourceAttributeDescription : _vertexBuffers[i].attributeDescriptions) {
            vk::VertexInputAttributeDescription attributeDescription = {};
            attributeDescription.binding = i;
            attributeDescription.location = sourceAttributeDescription.location;
            attributeDescription.format = VulkanEnums::formatToVulkan(sourceAttributeDescription.format);
            attributeDescription.offset = sourceAttributeDescription.offset;
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
    multisampling.setRasterizationSamples(VulkanEnums::msaaToVulkan(_renderPass->msaa()));

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
    graphicsPipelineInfo.setRenderPass(_renderPass->renderPassId());
    graphicsPipelineInfo.setSubpass(0);

    // create the graphics pipeline
    vk::Result result;
    std::tie(result, _graphicsPipeline) = VulkanContext::device.createGraphicsPipeline(nullptr, graphicsPipelineInfo);
    if (result != vk::Result::eSuccess)
        throw RendererError("Failed to create graphics pipeline");

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    VulkanUtils::setDebugObjectName(_graphicsPipeline, _name);
#endif // VULKAN_ENABLE_DEBUG_MARKER
}

void VulkanPipeline::cleanup()
{
    CHRZONE_RENDERER;

    // add data to destroy to the garbage collector
    VulkanGC::add(_graphicsPipeline);
    VulkanGC::add(_pipelineLayout);

    // descriptor sets clear
    _descriptorSets.clear();
}

void VulkanPipeline::debugShowLines([[maybe_unused]] const DebugShowLinesEvent& evn)
{
    // after a debug show line event, cleanup and recreate the pipeline
    cleanup();
    create();
}

std::vector<vk::DescriptorSetLayout> VulkanPipeline::getVulkanDescriptorSetsLayout(
    const std::vector<DescriptorSetLayout>& descriptorSetsLayout) const
{
    CHRZONE_RENDERER;

    // reserve descriptor sets layout memory
    std::vector<vk::DescriptorSetLayout> vulkanDescriptorSetsLayout;
    vulkanDescriptorSetsLayout.resize(descriptorSetsLayout.size());

    // create the descriptor set layouts
    for (uint32_t i = 0; i < descriptorSetsLayout.size(); i++) {
        const auto& shaderDescriptorSetLayout = descriptorSetsLayout[i];
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
        for (const auto& [_, shaderBinding] : shaderDescriptorSetLayout.bindings) {
            vk::DescriptorSetLayoutBinding binding = {};
            binding.setBinding(shaderBinding.binding);
            binding.setDescriptorType(VulkanEnums::descriptorTypeFromVulkan(shaderBinding.descriptorType));
            binding.setDescriptorCount(1); // TODO: this is the array size?
            binding.setStageFlags(VulkanEnums::shaderStageFlagsToVulkan(shaderBinding.stages));
            bindings.push_back(binding);
        }

        // create the descriptor set layout
        vk::DescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.setBindings(bindings);
        vulkanDescriptorSetsLayout[i] = VulkanContext::device.createDescriptorSetLayout(createInfo);
    }

    // return descriptor set layouts
    return vulkanDescriptorSetsLayout;
}

} // namespace chronicle