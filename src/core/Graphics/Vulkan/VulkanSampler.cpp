// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanSampler.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace chronicle::graphics::internal::vulkan
{

VulkanSampler::VulkanSampler(std::shared_ptr<VulkanDevice> device, const SamplerDescriptor &samplerDescriptor)
    : Sampler(samplerDescriptor), _device(device)
{
    // Create sampler.
    try
    {
        const auto properties = _device->vulkanPhysicalDevice().getProperties();

        auto maxSamplerAnisotropy =
            std::clamp(static_cast<float>(descriptor().maxAnisotropy), 1.0f, properties.limits.maxSamplerAnisotropy);

        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.setMagFilter(convertFilter(descriptor().magFilter));
        samplerInfo.setMinFilter(convertFilter(descriptor().minFilter));
        samplerInfo.setAddressModeU(convertSamplerAddressMode(descriptor().addressModeU));
        samplerInfo.setAddressModeV(convertSamplerAddressMode(descriptor().addressModeV));
        samplerInfo.setAddressModeW(convertSamplerAddressMode(descriptor().addressModeW));
        samplerInfo.setAnisotropyEnable(true);
        samplerInfo.setMaxAnisotropy(maxSamplerAnisotropy);
        samplerInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
        samplerInfo.setUnnormalizedCoordinates(false);
        samplerInfo.setCompareEnable(descriptor().compare.has_value());
        if (descriptor().compare.has_value())
        {
            samplerInfo.setCompareOp(converCompareOp(descriptor().compare.value()));
        }
        samplerInfo.setMipmapMode(convertSamplerMipmap(descriptor().mipmapFilter));
        samplerInfo.setMipLodBias(0.0f);
        samplerInfo.setMinLod(descriptor().lodMinClamp);
        samplerInfo.setMaxLod(descriptor().lodMaxClamp);

        _vulkanSampler = _device->vulkanLogicalDevice().createSampler(samplerInfo);
    }
    catch (const vk::Error &error)
    {
        throw SamplerError(fmt::format("Can't create sampler: {}", error.what()));
    }

#ifdef VULKAN_ENABLE_DEBUG_MARKER
    device->setDebugObjectName(vk::ObjectType::eSampler, (uint64_t)(VkSampler)_vulkanSampler, descriptor().name);
#endif
}

VulkanSampler::~VulkanSampler()
{
    _device->vulkanLogicalDevice().destroySampler(_vulkanSampler);
}

} // namespace chronicle::graphics::internal::vulkan