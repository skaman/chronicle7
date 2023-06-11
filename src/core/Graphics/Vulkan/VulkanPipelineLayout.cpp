// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanPipelineLayout.h"

#include "VulkanBindGroupLayout.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace chronicle::graphics::internal::vulkan
{

VulkanPipelineLayout::VulkanPipelineLayout(std::shared_ptr<VulkanDevice> device,
                                           const PipelineLayoutDescriptor &pipelineLayoutDescriptor)
    : PipelineLayout(pipelineLayoutDescriptor), _device(device)
{
    const auto &bindGroupLayouts = descriptor().bindGroupLayouts;
    _descriptorSetLayouts.resize(bindGroupLayouts.size());

    for (auto i = 0; i < bindGroupLayouts.size(); i++)
    {
        const auto &bindGroupLayout = static_cast<VulkanBindGroupLayout *>(bindGroupLayouts[i].get());
        _descriptorSetLayouts[i] = bindGroupLayout->vulkanDescriptorSetLayout();
    }
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
}

} // namespace chronicle::graphics::internal::vulkan