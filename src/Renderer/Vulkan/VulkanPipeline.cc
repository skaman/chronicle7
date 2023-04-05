#include "VulkanPipeline.h"

#include "VulkanCommon.h"

#include <../vendor/spirv-reflect/spirv_reflect.h>

namespace chronicle {

VulkanPipeline::VulkanPipeline(const vk::Device& device, const PipelineInfo& pipelineInfo)
    : _device(device)
{
    CHRZONE_VULKAN

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
    auto descriptorSetsLayoutData = getDescriptorSetsLayout(pipelineInfo.shaders);

    _descriptorSetsLayout.reserve(descriptorSetsLayoutData.size());

    for (const auto& descriptorSetLayoutData : descriptorSetsLayoutData) {
        vk::DescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.setBindings(descriptorSetLayoutData.bindings);
        _descriptorSetsLayout.push_back(_device.createDescriptorSetLayout(createInfo));
    }

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setSetLayouts(_descriptorSetsLayout);

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
    CHRZONE_VULKAN

    _device.destroyPipeline(_graphicsPipeline);
    _device.destroyPipelineLayout(_pipelineLayout);

    for (const auto& descriptorSetLayout : _descriptorSetsLayout)
        _device.destroyDescriptorSetLayout(descriptorSetLayout);
}

vk::ShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) const
{
    CHRZONE_VULKAN

    return _device.createShaderModule(
        { vk::ShaderModuleCreateFlags(), code.size(), std::bit_cast<const uint32_t*>(code.data()) });
}

std::vector<DescriptorSetLayoutData> VulkanPipeline::getDescriptorSetsLayout(
    const std::unordered_map<ShaderStage, std::vector<char>>& shaders) const
{
    CHRZONE_VULKAN

    std::map<uint32_t, DescriptorSetLayoutData> sets = {};
    for (auto const& [shaderStage, code] : shaders) {
        auto descriptorSetsLayoutData = getDescriptorSetsLayout(code);
        for (const auto& descriptorSetLayoutData : descriptorSetsLayoutData) {
            if (!sets.contains(descriptorSetLayoutData.setNumber))
                sets[descriptorSetLayoutData.setNumber] = descriptorSetLayoutData;
        }
    }

    std::vector<DescriptorSetLayoutData> result = {};
    for (auto const& [setNumber, descriptorSetLayoutData] : sets) {
        result.push_back(descriptorSetLayoutData);
    }

    return result;
}

std::vector<DescriptorSetLayoutData> VulkanPipeline::getDescriptorSetsLayout(const std::vector<char>& code) const
{
    CHRZONE_VULKAN

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