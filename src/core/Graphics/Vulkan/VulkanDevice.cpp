// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanDevice.h"

#include "VulkanBuffer.h"
#include "VulkanCommandEncoder.h"
#include "VulkanSampler.h"
#include "VulkanSystem.h"

namespace chronicle::graphics::internal::vulkan
{

VkResult setDebugUtilsObjectNameEXT(VkInstance instance, VkDevice device,
                                    const VkDebugUtilsObjectNameInfoEXT *pNameInfo)
{
    auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    if (func != nullptr)
        return func(device, pNameInfo);

    return VK_ERROR_UNKNOWN;
}

VulkanDevice::VulkanDevice(vk::PhysicalDevice physicalDevice, const VulkanQueueFamilyIndices &indices)
    : _vulkanPhysicalDevice(physicalDevice)
{
    // prepare the device create info for every family
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    std::array<float, 1> queuePriorities = {1.0f};
    for (auto queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, queueFamily, queuePriorities);
    }

    // get and enabled the device features
    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.setSamplerAnisotropy(true);
    deviceFeatures.setFillModeNonSolid(true);

    // create the logical device
    auto layers = VulkanSystem::enabledLayers();
    auto extensions = VulkanSystem::enabledExtensions();
    vk::DeviceCreateInfo createInfo(vk::DeviceCreateFlags{}, queueCreateInfos, layers, extensions, &deviceFeatures);
    _vulkanLogicalDevice = _vulkanPhysicalDevice.createDevice(createInfo);

    // store queues
    _vulkanGraphicsQueue = _vulkanLogicalDevice.getQueue(indices.graphicsFamily.value(), 0);
    _vulkanPresentQueue = _vulkanLogicalDevice.getQueue(indices.presentFamily.value(), 0);

    // create the command pool
    vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                    indices.graphicsFamily.value());
    _vulkanCommandPool = _vulkanLogicalDevice.createCommandPool(commandPoolCreateInfo);

    // create the descriptor pool
    std::array<vk::DescriptorPoolSize, 5> descriptorPoolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
        vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
        vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
        vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1000)};
    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, static_cast<uint32_t>(1000 * descriptorPoolSizes.size()),
                                                          descriptorPoolSizes);
    _vulkanDescriptorPool = _vulkanLogicalDevice.createDescriptorPool(descriptorPoolCreateInfo, nullptr);
}

VulkanDevice::~VulkanDevice()
{
    _vulkanLogicalDevice.destroyDescriptorPool(_vulkanDescriptorPool);
    _vulkanLogicalDevice.destroyCommandPool(_vulkanCommandPool);
    _vulkanLogicalDevice.destroy();
}

std::shared_ptr<CommandEncoder> VulkanDevice::createCommandEncoder(
    const CommandEncoderCreateInfo &commandEncoderCreateInfo) const
{
    return std::make_shared<VulkanCommandEncoder>(_thisWeakPtr.lock(), commandEncoderCreateInfo);
}

std::shared_ptr<Buffer> VulkanDevice::createBuffer(const BufferDescriptor &bufferDescriptor) const
{
    return std::make_shared<VulkanBuffer>(_thisWeakPtr.lock(), bufferDescriptor);
}

std::shared_ptr<Sampler> VulkanDevice::createSampler(const SamplerDescriptor &samplerDescriptor) const
{
    return std::make_shared<VulkanSampler>(_thisWeakPtr.lock(), samplerDescriptor);
}

void VulkanDevice::setDebugObjectName(vk::ObjectType objectType, uint64_t handle, const std::string &name) const

{
    if (name.empty())
        return;

    VkDebugUtilsObjectNameInfoEXT objectNameInfo = {};
    objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    objectNameInfo.objectType = (VkObjectType)objectType;
    objectNameInfo.objectHandle = handle;
    objectNameInfo.pObjectName = name.c_str();
    objectNameInfo.pNext = nullptr;

    setDebugUtilsObjectNameEXT(VulkanSystem::vulkanInstance(), _vulkanLogicalDevice, &objectNameInfo);
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    // get memory properties
    auto memProperties = _vulkanPhysicalDevice.getMemoryProperties();

    // get the first memory location with compatible flags
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw GraphicsError("Failed to find suitable memory type");
}

} // namespace chronicle::graphics::internal::vulkan