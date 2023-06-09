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
    : _physicalDevice(physicalDevice)
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
    _logicalDevice = _physicalDevice.createDevice(createInfo);

    // store queues
    _graphicsQueue = _logicalDevice.getQueue(indices.graphicsFamily.value(), 0);
    _presentQueue = _logicalDevice.getQueue(indices.presentFamily.value(), 0);

    // create the command pool
    vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                       indices.graphicsFamily.value());
    _commandPool = _logicalDevice.createCommandPool(poolInfo);
}

VulkanDevice::~VulkanDevice()
{
    _logicalDevice.destroyCommandPool(_commandPool);
    _logicalDevice.destroy();
}

std::shared_ptr<CommandEncoder> VulkanDevice::createCommandEncoder(
    const CommandEncoderCreateInfo &commandEncoderCreateInfo) const
{
    return std::make_shared<VulkanCommandEncoder>(_thisWeakPtr.lock(), commandEncoderCreateInfo);
}

std::shared_ptr<Buffer> VulkanDevice::createBuffer(const BufferCreateInfo &bufferCreateInfo) const
{
    return std::make_shared<VulkanBuffer>(_thisWeakPtr.lock(), bufferCreateInfo);
}

std::shared_ptr<Sampler> VulkanDevice::createSampler(const SamplerCreateInfo &samplerCreateInfo) const
{
    return std::make_shared<VulkanSampler>(_thisWeakPtr.lock(), samplerCreateInfo);
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

    setDebugUtilsObjectNameEXT(VulkanSystem::vulkanInstance(), _logicalDevice, &objectNameInfo);
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    // get memory properties
    auto memProperties = _physicalDevice.getMemoryProperties();

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