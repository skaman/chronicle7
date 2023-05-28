// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Renderer.h"

namespace chronicle::internal::vulkan {

/// @brief Entry types for garbage collector.
enum class GCType { pipeline, pipelineLayout, buffer, deviceMemory, descriptorSetLayout };

/// @brief Garbage collector data.
struct GCData {
    GCType type {}; ///< Entry type.
    union {
        vk::Pipeline pipeline; ///< Pipeline
        vk::PipelineLayout pipelineLayout; ///< Pipeline layout
        vk::Buffer buffer; ///< Buffer
        vk::DeviceMemory deviceMemory; ///< Device memory
        vk::DescriptorSetLayout descriptorSetLayout; ///< Descriptor set layout
    };

    explicit GCData(vk::Pipeline pipeline)
        : type(GCType::pipeline)
        , pipeline(pipeline)
    {
    }

    explicit GCData(vk::PipelineLayout pipelineLayout)
        : type(GCType::pipelineLayout)
        , pipelineLayout(pipelineLayout)
    {
    }

    explicit GCData(vk::Buffer buffer)
        : type(GCType::buffer)
        , buffer(buffer)
    {
    }

    explicit GCData(vk::DeviceMemory deviceMemory)
        : type(GCType::deviceMemory)
        , deviceMemory(deviceMemory)
    {
    }

    explicit GCData(vk::DescriptorSetLayout descriptorSetLayout)
        : type(GCType::descriptorSetLayout)
        , descriptorSetLayout(descriptorSetLayout)
    {
    }
};

struct VulkanGCContext {
    static inline std::vector<std::vector<GCData>> queues {};
};

class VulkanGC {
public:
    template <class T> static void add(const T& data) noexcept
    {
        try {
            if (VulkanGCContext::queues.empty()) {
                VulkanGCContext::queues.resize(RenderContext::maxFramesInFlight());
            }
            auto& queue = VulkanGCContext::queues[RenderContext::currentFrame()];
            queue.emplace_back(data);
        } catch (const std::exception& e) {
            CHRLOG_ERROR("Memory leak: {}", e.what());
        }
    }

    static void cleanupCurrentQueue()
    {
        if (VulkanGCContext::queues.empty())
            return;

        auto& queue = VulkanGCContext::queues[RenderContext::currentFrame()];
        cleanup(queue);
    }

    static void cleanupAll()
    {
        for (auto& queue : VulkanGCContext::queues) {
            cleanup(queue);
        }
    }

private:
    static void cleanup(std::vector<GCData>& queue)
    {
        // cycle all items and clean them
        for (const auto& item : queue) {
            switch (item.type) {
            case GCType::pipeline:
                VulkanContext::device.destroyPipeline(item.pipeline);
                break;
            case GCType::pipelineLayout:
                VulkanContext::device.destroyPipelineLayout(item.pipelineLayout);
                break;
            case GCType::buffer:
                VulkanContext::device.destroyBuffer(item.buffer);
                break;
            case GCType::deviceMemory:
                VulkanContext::device.freeMemory(item.deviceMemory);
                break;
            case GCType::descriptorSetLayout:
                VulkanContext::device.destroyDescriptorSetLayout(item.descriptorSetLayout);
                break;
            default:
                break;
            }
        }

        // clear the vector
        queue.clear();
    }
};

} // namespace chronicle