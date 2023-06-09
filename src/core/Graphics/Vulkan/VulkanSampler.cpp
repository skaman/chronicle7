// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanSampler.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace chronicle::graphics::internal::vulkan
{

VulkanSampler::VulkanSampler(std::shared_ptr<VulkanDevice> device, const SamplerCreateInfo &samplerCreateInfo)
    : _device(device)
{
    const auto properties = _device->vulkanPhysicalDevice().getProperties();

    auto maxSamplerAnisotropy =
        std::clamp(static_cast<float>(samplerCreateInfo.maxAnisotropy), 1.0f, properties.limits.maxSamplerAnisotropy);

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(convertFilter(samplerCreateInfo.magFilter));
    samplerInfo.setMinFilter(convertFilter(samplerCreateInfo.minFilter));
    samplerInfo.setAddressModeU(convertSamplerAddressMode(samplerCreateInfo.addressModeU));
    samplerInfo.setAddressModeV(convertSamplerAddressMode(samplerCreateInfo.addressModeV));
    samplerInfo.setAddressModeW(convertSamplerAddressMode(samplerCreateInfo.addressModeW));
    samplerInfo.setAnisotropyEnable(true);
    samplerInfo.setMaxAnisotropy(maxSamplerAnisotropy);
    samplerInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
    samplerInfo.setUnnormalizedCoordinates(false);
    samplerInfo.setCompareEnable(samplerCreateInfo.compare.has_value());
    if (samplerCreateInfo.compare.has_value())
    {
        samplerInfo.setCompareOp(converCompareOp(samplerCreateInfo.compare.value()));
    }
    samplerInfo.setMipmapMode(convertSamplerMipmap(samplerCreateInfo.mipmapFilter));
    samplerInfo.setMipLodBias(0.0f);
    samplerInfo.setMinLod(samplerCreateInfo.lodMinClamp);
    samplerInfo.setMaxLod(samplerCreateInfo.lodMaxClamp);
    _vulkanSampler = _device->vulkanLogicalDevice().createSampler(samplerInfo);

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eSampler, (uint64_t)(VkSampler)_vulkanSampler, _name);
#endif
}

VulkanSampler::~VulkanSampler()
{
    _device->vulkanLogicalDevice().destroySampler(_vulkanSampler);
}

} // namespace chronicle::graphics::internal::vulkan