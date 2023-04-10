#include "VulkanImageUtils.h"

#include "VulkanBuffer.h"

namespace chronicle {

std::pair<vk::DeviceMemory, vk::Image> VulkanImageUtils::createImage(vk::Device device,
    vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format,
    vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    CHRZONE_VULKAN

    vk::Extent3D imageExtent = {};
    imageExtent.setWidth(width);
    imageExtent.setHeight(height);
    imageExtent.setDepth(1);

    vk::ImageCreateInfo imageCreateInfo {};
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setExtent(imageExtent);
    imageCreateInfo.setMipLevels(mipLevels);
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setFormat(format);
    imageCreateInfo.setTiling(tiling);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setUsage(usage);
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

    auto image = device.createImage(imageCreateInfo);

    // allocate memory
    const auto memRequirements = device.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.setAllocationSize(memRequirements.size);
    allocInfo.setMemoryTypeIndex(
        VulkanBuffer::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties));

    auto imageMemory = device.allocateMemory(allocInfo, nullptr);

    vkBindImageMemory(device, image, imageMemory, 0);

    return { imageMemory, image };
}

vk::ImageView VulkanImageUtils::createImageView(
    vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    CHRZONE_VULKAN

    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.setAspectMask(aspectFlags);
    subresourceRange.setBaseMipLevel(0);
    subresourceRange.setLevelCount(1);
    subresourceRange.setBaseArrayLayer(0);
    subresourceRange.setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.setSubresourceRange(subresourceRange);

    return device.createImageView(viewInfo);
}

vk::Sampler VulkanImageUtils::createTextureSampler(vk::Device device, vk::PhysicalDevice physicalDevice)
{
    CHRZONE_VULKAN

    const auto properties = physicalDevice.getProperties();

    vk::SamplerCreateInfo samplerInfo = {};
    samplerInfo.setMagFilter(vk::Filter::eLinear);
    samplerInfo.setMinFilter(vk::Filter::eLinear);
    samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAnisotropyEnable(true);
    samplerInfo.setMaxAnisotropy(properties.limits.maxSamplerAnisotropy);
    samplerInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
    samplerInfo.setUnnormalizedCoordinates(false);
    samplerInfo.setCompareEnable(false);
    samplerInfo.setCompareOp(vk::CompareOp::eAlways);
    samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    samplerInfo.setMipLodBias(0.0f);
    samplerInfo.setMinLod(0.0f);
    samplerInfo.setMaxLod(0.0f);

    return device.createSampler(samplerInfo);
}

} // namespace chronicle