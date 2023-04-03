#include "VulkanPipeline.h"

#include "VulkanCommon.h"

namespace chronicle {

VulkanPipeline::VulkanPipeline(const vk::Device& device, const PipelineInfo& pipelineInfo)
    : _device(device)
{
    // shader stages
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(pipelineInfo.shaders.size());

    std::vector<vk::ShaderModule> shaderModules;
    shaderModules.reserve(pipelineInfo.shaders.size());

    for (auto const& [shaderStage, code] : pipelineInfo.shaders) {
        auto shaderModule = createShaderModule(code);
        vk::PipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.setStage(shaderStageToVulkan(shaderStage));
        shaderStageCreateInfo.setModule(shaderModule);
        shaderStageCreateInfo.setPName("main");

        shaderStages.push_back(shaderStageCreateInfo);
        shaderModules.push_back(shaderModule);
    }

    // create binding and attribute descriptions for vertex input state
    auto attributeDescriptionsCount = 0;
    for (const auto& vertexBufferInfo : pipelineInfo.vertexBuffers)
        attributeDescriptionsCount += vertexBufferInfo.attributeDescriptions.size();

    std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {};
    bindingDescriptions.reserve(pipelineInfo.vertexBuffers.size());

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};
    attributeDescriptions.reserve(attributeDescriptionsCount);

    for (auto i = 0; i < pipelineInfo.vertexBuffers.size(); i++) {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = i;
        bindingDescription.stride = pipelineInfo.vertexBuffers[i].stride;
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        bindingDescriptions.push_back(bindingDescription);

        for (auto j = 0; j < pipelineInfo.vertexBuffers[i].attributeDescriptions.size(); j++) {
            vk::VertexInputAttributeDescription attributeDescription = {};
            attributeDescription.binding = i;
            attributeDescription.location = j;
            attributeDescription.format = formatToVulkan(pipelineInfo.vertexBuffers[i].attributeDescriptions[j].format);
            attributeDescription.offset = pipelineInfo.vertexBuffers[i].attributeDescriptions[j].offset;
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
    rasterizer.setPolygonMode(vk::PolygonMode::eFill);
    rasterizer.setLineWidth(1.0f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);
    rasterizer.setDepthBiasEnable(false);

    // multisample state
    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.setSampleShadingEnable(false);
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);

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

    // descriptor sets layout
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetsLayout = {};
    for (const auto& descriptorSet : pipelineInfo.descriptorSets) {
        const auto& layoutBindings = descriptorSet->native().layoutBindings();
        for (const auto& layoutBinding : layoutBindings) {
            // TODO: handle the binding value?
            descriptorSetsLayout.push_back(layoutBinding);
        }
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.setBindings(descriptorSetsLayout);

    _descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setSetLayouts(_descriptorSetLayout);

    _pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);

    // graphics pipeline
    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.setStages(shaderStages);
    graphicsPipelineInfo.setPVertexInputState(&vertexInputInfo);
    graphicsPipelineInfo.setPInputAssemblyState(&inputAssembly);
    graphicsPipelineInfo.setPViewportState(&viewportState);
    graphicsPipelineInfo.setPRasterizationState(&rasterizer);
    graphicsPipelineInfo.setPMultisampleState(&multisampling);
    graphicsPipelineInfo.setPColorBlendState(&colorBlending);
    graphicsPipelineInfo.setPDynamicState(&dynamicState);
    graphicsPipelineInfo.setLayout(_pipelineLayout);
    graphicsPipelineInfo.setRenderPass(pipelineInfo.renderPass->native().renderPass());
    graphicsPipelineInfo.setSubpass(0);

    vk::Result result;
    std::tie(result, _graphicsPipeline) = _device.createGraphicsPipeline(nullptr, graphicsPipelineInfo);
    if (result != vk::Result::eSuccess)
        throw RendererError("Failed to create graphics pipeline");

    for (auto const& shaderModule : shaderModules)
        _device.destroyShaderModule(shaderModule);
}

VulkanPipeline::~VulkanPipeline()
{
    _device.destroyPipeline(_graphicsPipeline);
    _device.destroyPipelineLayout(_pipelineLayout);
    _device.destroyDescriptorSetLayout(_descriptorSetLayout);
}

vk::ShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) const
{
    return _device.createShaderModule(
        { vk::ShaderModuleCreateFlags(), code.size(), std::bit_cast<const uint32_t*>(code.data()) });
}

} // namespace chronicle