// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief vkCreateDebugUtilsMessengerEXT.
/// @param instance The instance the messenger will be used with.
/// @param pCreateInfo A pointer to a VkDebugUtilsMessengerCreateInfoEXT structure containing the callback pointer, as
///                    well as defining conditions under which this messenger will trigger the callback.
/// @param pAllocator Controls host memory allocation as described in the Memory Allocation chapter.
/// @param pCallback A pointer to a VkDebugUtilsMessengerEXT handle in which the created object is returned.
/// @return On success VK_SUCCESS, on failure VK_ERROR_OUT_OF_HOST_MEMORY.
inline VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    return func != nullptr ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

/// @brief Destroy a debug messenger object.
/// @param instance The instance where the callback was created.
/// @param callback The VkDebugUtilsMessengerEXT object to destroy. messenger is an externally synchronized object and
///                 must not be used on more than one thread at a time. This means that vkDestroyDebugUtilsMessengerEXT
///                 must not be called when a callback is active.
/// @param pAllocator Controls host memory allocation as described in the Memory Allocation chapter.
inline void destroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, callback, pAllocator);
}

/// @brief Give a user-friendly name to an object.
/// @param instance The instance.
/// @param device The device that is associated with the named object passed in via objectHandle.
/// @param pNameInfo A pointer to a VkDebugUtilsObjectNameInfoEXT structure specifying parameters of the name to set on
///                  the object.
/// @return On success VK_SUCCESS, otherwise VK_ERROR_OUT_OF_HOST_MEMORY, VK_ERROR_OUT_OF_DEVICE_MEMORY and
///         VK_ERROR_UNKNOWN.
inline VkResult setDebugUtilsObjectNameEXT(
    VkInstance instance, VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
    auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    if (func != nullptr)
        return func(device, pNameInfo);

    return VK_ERROR_UNKNOWN;
}

/// @brief Open a command buffer debug label region.
/// @param instance The instance.
/// @param commandBuffer The command buffer into which the command is recorded.
/// @param pLabelInfo A pointer to a VkDebugUtilsLabelEXT structure specifying parameters of the label region to open.
inline void cmdBeginDebugUtilsLabelEXT(
    VkInstance instance, VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
{
    auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    if (func != nullptr)
        func(commandBuffer, pLabelInfo);
}

/// @brief Close a command buffer label region.
/// @param instance The instance.
/// @param commandBuffer The command buffer into which the command is recorded.
inline void cmdEndDebugUtilsLabelEXT(
    VkInstance instance, VkCommandBuffer commandBuffer)
{
    auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    if (func != nullptr)
        func(commandBuffer);
}

/// @brief Insert a label into a command buffer.
/// @param instance The instance.
/// @param commandBuffer The command buffer into which the command is recorded.
/// @param pLabelInfo A pointer to a VkDebugUtilsLabelEXT structure specifying parameters of the label to insert.
inline void cmdInsertDebugUtilsLabelEXT(
    VkInstance instance, VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
{
    auto func = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
    if (func != nullptr)
        func(commandBuffer, pLabelInfo);
}

} // namespace chronicle