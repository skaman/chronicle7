#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/RendererError.h"

namespace chronicle {

inline vk::ShaderStageFlagBits shaderStageToVulkan(ShaderStage stage)
{
    switch (stage) {
    case ShaderStage::Fragment:
        return vk::ShaderStageFlagBits::eFragment;
    case ShaderStage::Vertex:
        return vk::ShaderStageFlagBits::eVertex;
    default:
        throw RendererError("Unsupported shader stage");
    }
}

inline vk::Format formatToVulkan(Format format)
{
    switch (format) {
    case Format::Undefined:
        return vk::Format::eUndefined;
    case Format::B8G8R8A8Unorm:
        return vk::Format::eB8G8R8A8Unorm;
    case Format::R32G32Sfloat:
        return vk::Format::eR32G32Sfloat;
    case Format::R32G32B32Sfloat:
        return vk::Format::eR32G32B32Sfloat;
    case Format::D32Sfloat:
        return vk::Format::eD32Sfloat;
    case Format::D32SfloatS8Uint:
        return vk::Format::eD32SfloatS8Uint;
    case Format::D24UnormS8Uint:
        return vk::Format::eD24UnormS8Uint;
    default:
        throw RendererError("Unsupported format");
    }
}

inline Format formatFromVulkan(vk::Format format)
{
    switch (format) {
    case vk::Format::eUndefined:
        return Format::Undefined;
    case vk::Format::eB8G8R8A8Unorm:
        return Format::B8G8R8A8Unorm;
    case vk::Format::eR32G32Sfloat:
        return Format::R32G32Sfloat;
    case vk::Format::eR32G32B32Sfloat:
        return Format::R32G32B32Sfloat;
    case vk::Format::eD32Sfloat:
        return Format::D32Sfloat;
    case vk::Format::eD32SfloatS8Uint:
        return Format::D32SfloatS8Uint;
    case vk::Format::eD24UnormS8Uint:
        return Format::D24UnormS8Uint;
    default:
        throw RendererError("Unsupported format");
    }
}

inline vk::VertexInputRate vertextInputRateToVulkan(VertexInputRate vertexInputRate)
{
    switch (vertexInputRate) {
    case VertexInputRate::Vertex:
        return vk::VertexInputRate::eVertex;
    case VertexInputRate::Instance:
        return vk::VertexInputRate::eInstance;
    default:
        throw RendererError("Unsupported vertex input rate");
    }
}

//inline vk::DescriptorType descriptorTypeToVulkan(DescriptorType descriptorType)
//{
//    switch (descriptorType) {
//    case DescriptorType::UniformBuffer:
//        return vk::DescriptorType::eUniformBuffer;
//    default:
//        throw RendererError("Unsupported descriptor type");
//    }
//}

} // namespace chronicle