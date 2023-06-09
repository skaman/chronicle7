// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "VulkanCommon.h"

namespace chronicle::internal::vulkan {

class VulkanEnums {
public:
    static vk::ShaderStageFlags shaderStageFlagsToVulkan(ShaderStage stage)
    {
        vk::ShaderStageFlags result = {};
        if (!!(stage & ShaderStage::fragment))
            result |= vk::ShaderStageFlagBits::eFragment;
        if (!!(stage & ShaderStage::vertex))
            result |= vk::ShaderStageFlagBits::eVertex;
        if (!!(stage & ShaderStage::compute))
            result |= vk::ShaderStageFlagBits::eCompute;
        return result;
    }

    static vk::ShaderStageFlagBits shaderStageToVulkan(ShaderStage stage)
    {
        switch (stage) {
        case ShaderStage::fragment:
            return vk::ShaderStageFlagBits::eFragment;
        case ShaderStage::vertex:
            return vk::ShaderStageFlagBits::eVertex;
        case ShaderStage::compute:
            return vk::ShaderStageFlagBits::eCompute;
        default:
            throw RendererError("Unsupported stage flags");
        }
    }

    static vk::IndexType indexTypeToVulkan(IndexType indexType)
    {
        switch (indexType) {
        case IndexType::uint16:
            return vk::IndexType::eUint16;
        case IndexType::uint32:
            return vk::IndexType::eUint32;
        default:
            throw RendererError("Unsupported index type");
        }
    }

    static vk::Format formatToVulkan(Format format)
    {
        switch (format) {
        case Format::undefined:
            return vk::Format::eUndefined;

            // 8 bit signed int
        case Format::R8Sint:
            return vk::Format::eR8Sint;
        case Format::R8G8Sint:
            return vk::Format::eR8G8Sint;
        case Format::R8G8B8Sint:
            return vk::Format::eR8G8B8Sint;
        case Format::R8G8B8A8Sint:
            return vk::Format::eR8G8B8A8Sint;

            // 8 bit unsigned int
        case Format::R8Uint:
            return vk::Format::eR8Uint;
        case Format::R8G8Uint:
            return vk::Format::eR8G8Uint;
        case Format::R8G8B8Uint:
            return vk::Format::eR8G8B8Uint;
        case Format::R8G8B8A8Uint:
            return vk::Format::eR8G8B8A8Uint;

            // 8 bit normalized
        case Format::R8Unorm:
            return vk::Format::eR8Unorm;
        case Format::R8G8Unorm:
            return vk::Format::eR8G8Unorm;
        case Format::R8G8B8Unorm:
            return vk::Format::eR8G8B8Unorm;
        case Format::R8G8B8A8Unorm:
            return vk::Format::eR8G8B8A8Unorm;

            // 16 bit signed int
        case Format::R16Sint:
            return vk::Format::eR16Sint;
        case Format::R16G16Sint:
            return vk::Format::eR16G16Sint;
        case Format::R16G16B16Sint:
            return vk::Format::eR16G16B16Sint;
        case Format::R16G16B16A16Sint:
            return vk::Format::eR16G16B16A16Sint;

            // 16 bit unsigned int
        case Format::R16Uint:
            return vk::Format::eR16Uint;
        case Format::R16G16Uint:
            return vk::Format::eR16G16Uint;
        case Format::R16G16B16Uint:
            return vk::Format::eR16G16B16Uint;
        case Format::R16G16B16A16Uint:
            return vk::Format::eR16G16B16A16Uint;

            // 16 bit normalized
        case Format::R16Unorm:
            return vk::Format::eR16Unorm;
        case Format::R16G16Unorm:
            return vk::Format::eR16G16Unorm;
        case Format::R16G16B16Unorm:
            return vk::Format::eR16G16B16Unorm;
        case Format::R16G16B16A16Unorm:
            return vk::Format::eR16G16B16A16Unorm;

            // 32 bit signed int
        case Format::R32Sint:
            return vk::Format::eR32Sint;
        case Format::R32G32Sint:
            return vk::Format::eR32G32Sint;
        case Format::R32G32B32Sint:
            return vk::Format::eR32G32B32Sint;
        case Format::R32G32B32A32Sint:
            return vk::Format::eR32G32B32A32Sint;

            // 32 bit unsigned int
        case Format::R32Uint:
            return vk::Format::eR32Uint;
        case Format::R32G32Uint:
            return vk::Format::eR32G32Uint;
        case Format::R32G32B32Uint:
            return vk::Format::eR32G32B32Uint;
        case Format::R32G32B32A32Uint:
            return vk::Format::eR32G32B32A32Uint;

            // 32 bit signed float
        case Format::R32Sfloat:
            return vk::Format::eR32Sfloat;
        case Format::R32G32Sfloat:
            return vk::Format::eR32G32Sfloat;
        case Format::R32G32B32Sfloat:
            return vk::Format::eR32G32B32Sfloat;
        case Format::R32G32B32A32Sfloat:
            return vk::Format::eR32G32B32A32Sfloat;

        case Format::B8G8R8A8Unorm:
            return vk::Format::eB8G8R8A8Unorm;
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

    static Format formatFromVulkan(vk::Format format)
    {
        switch (format) {
        case vk::Format::eUndefined:
            return Format::undefined;

            // 8 bit signed int
        case vk::Format::eR8Sint:
            return Format::R8Sint;
        case vk::Format::eR8G8Sint:
            return Format::R8G8Sint;
        case vk::Format::eR8G8B8Sint:
            return Format::R8G8B8Sint;
        case vk::Format::eR8G8B8A8Sint:
            return Format::R8G8B8A8Sint;

            // 8 bit unsigned int
        case vk::Format::eR8Uint:
            return Format::R8Uint;
        case vk::Format::eR8G8Uint:
            return Format::R8G8Uint;
        case vk::Format::eR8G8B8Uint:
            return Format::R8G8B8Uint;
        case vk::Format::eR8G8B8A8Uint:
            return Format::R8G8B8A8Uint;

            // 8 bit normalized
        case vk::Format::eR8Unorm:
            return Format::R8Unorm;
        case vk::Format::eR8G8Unorm:
            return Format::R8G8Unorm;
        case vk::Format::eR8G8B8Unorm:
            return Format::R8G8B8Unorm;
        case vk::Format::eR8G8B8A8Unorm:
            return Format::R8G8B8A8Unorm;

            // 16 bit signed int
        case vk::Format::eR16Sint:
            return Format::R16Sint;
        case vk::Format::eR16G16Sint:
            return Format::R16G16Sint;
        case vk::Format::eR16G16B16Sint:
            return Format::R16G16B16Sint;
        case vk::Format::eR16G16B16A16Sint:
            return Format::R16G16B16A16Sint;

            // 16 bit unsigned int
        case vk::Format::eR16Uint:
            return Format::R16Uint;
        case vk::Format::eR16G16Uint:
            return Format::R16G16Uint;
        case vk::Format::eR16G16B16Uint:
            return Format::R16G16B16Uint;
        case vk::Format::eR16G16B16A16Uint:
            return Format::R16G16B16A16Uint;

            // 16 bit normalized
        case vk::Format::eR16Unorm:
            return Format::R16Unorm;
        case vk::Format::eR16G16Unorm:
            return Format::R16G16Unorm;
        case vk::Format::eR16G16B16Unorm:
            return Format::R16G16B16Unorm;
        case vk::Format::eR16G16B16A16Unorm:
            return Format::R16G16B16A16Unorm;

            // 32 bit signed int
        case vk::Format::eR32Sint:
            return Format::R32Sint;
        case vk::Format::eR32G32Sint:
            return Format::R32G32Sint;
        case vk::Format::eR32G32B32Sint:
            return Format::R32G32B32Sint;
        case vk::Format::eR32G32B32A32Sint:
            return Format::R32G32B32A32Sint;

            // 32 bit unsigned int
        case vk::Format::eR32Uint:
            return Format::R32Uint;
        case vk::Format::eR32G32Uint:
            return Format::R32G32Uint;
        case vk::Format::eR32G32B32Uint:
            return Format::R32G32B32Uint;
        case vk::Format::eR32G32B32A32Uint:
            return Format::R32G32B32A32Uint;

            // 32 bit signed float
        case vk::Format::eR32Sfloat:
            return Format::R32Sfloat;
        case vk::Format::eR32G32Sfloat:
            return Format::R32G32Sfloat;
        case vk::Format::eR32G32B32Sfloat:
            return Format::R32G32B32Sfloat;
        case vk::Format::eR32G32B32A32Sfloat:
            return Format::R32G32B32A32Sfloat;

        case vk::Format::eB8G8R8A8Unorm:
            return Format::B8G8R8A8Unorm;
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

    static vk::DescriptorType descriptorTypeFromVulkan(DescriptorType descriptorType)
    {
        switch (descriptorType) {
        case DescriptorType::sampler:
            return vk::DescriptorType::eSampler;
        case DescriptorType::combinedImageSampler:
            return vk::DescriptorType::eCombinedImageSampler;
        case DescriptorType::sampledImage:
            return vk::DescriptorType::eSampledImage;
        case DescriptorType::storageImage:
            return vk::DescriptorType::eStorageImage;
        case DescriptorType::uniformTexelBuffer:
            return vk::DescriptorType::eUniformTexelBuffer;
        case DescriptorType::storageTexelBuffer:
            return vk::DescriptorType::eStorageTexelBuffer;
        case DescriptorType::uniformBuffer:
            return vk::DescriptorType::eUniformBuffer;
        case DescriptorType::storageBuffer:
            return vk::DescriptorType::eStorageBuffer;
        case DescriptorType::uniformBufferDynamic:
            return vk::DescriptorType::eUniformBufferDynamic;
        case DescriptorType::storageBufferDynamic:
            return vk::DescriptorType::eStorageBufferDynamic;
        case DescriptorType::inputAttachment:
            return vk::DescriptorType::eInputAttachment;
        case DescriptorType::accelerationStructure:
            return vk::DescriptorType::eAccelerationStructureKHR;
        default:
            throw RendererError("Unsupported descriptor type");
        }
    }

    static vk::SampleCountFlagBits msaaToVulkan(MSAA msaa)
    {
        switch (msaa) {
        case MSAA::sampleCount1:
            return vk::SampleCountFlagBits::e1;
        case MSAA::sampleCount2:
            return vk::SampleCountFlagBits::e2;
        case MSAA::sampleCount4:
            return vk::SampleCountFlagBits::e4;
        case MSAA::sampleCount8:
            return vk::SampleCountFlagBits::e8;
        case MSAA::sampleCount16:
            return vk::SampleCountFlagBits::e16;
        case MSAA::sampleCount32:
            return vk::SampleCountFlagBits::e32;
        case MSAA::sampleCount64:
            return vk::SampleCountFlagBits::e64;
        default:
            throw RendererError("Unsupported MSAA");
        }
    }

    static MSAA msaaFromVulkan(vk::SampleCountFlagBits sampleCountFlagBit)
    {
        switch (sampleCountFlagBit) {
        case vk::SampleCountFlagBits::e1:
            return MSAA::sampleCount1;
        case vk::SampleCountFlagBits::e2:
            return MSAA::sampleCount2;
        case vk::SampleCountFlagBits::e4:
            return MSAA::sampleCount4;
        case vk::SampleCountFlagBits::e8:
            return MSAA::sampleCount8;
        case vk::SampleCountFlagBits::e16:
            return MSAA::sampleCount16;
        case vk::SampleCountFlagBits::e32:
            return MSAA::sampleCount32;
        case vk::SampleCountFlagBits::e64:
            return MSAA::sampleCount64;
        default:
            throw RendererError("Unsupported MSAA");
        }
    }

    static vk::AttachmentLoadOp attachmentLoadOpToVulkan(AttachmentLoadOp attachmentLoadOp)
    {
        switch (attachmentLoadOp) {
        case AttachmentLoadOp::load:
            return vk::AttachmentLoadOp::eLoad;
        case AttachmentLoadOp::clear:
            return vk::AttachmentLoadOp::eClear;
        case AttachmentLoadOp::dontCare:
            return vk::AttachmentLoadOp::eDontCare;
        default:
            throw RendererError("Unsupported attachment load operation");
        }
    }

    static vk::AttachmentStoreOp attachmentStoreOpToVulkan(AttachmentStoreOp attachmentStoreOp)
    {
        switch (attachmentStoreOp) {
        case AttachmentStoreOp::store:
            return vk::AttachmentStoreOp::eStore;
        case AttachmentStoreOp::dontCare:
            return vk::AttachmentStoreOp::eDontCare;
        default:
            throw RendererError("Unsupported attachment store operation");
        }
    }

    static vk::ImageLayout imageLayoutToVulkan(ImageLayout imageLayout)
    {
        switch (imageLayout) {
        case ImageLayout::undefined:
            return vk::ImageLayout::eUndefined;
        case ImageLayout::colorAttachment:
            return vk::ImageLayout::eColorAttachmentOptimal;
        case ImageLayout::depthStencilAttachment:
            return vk::ImageLayout::eDepthStencilAttachmentOptimal;
        case ImageLayout::shaderReadOnly:
            return vk::ImageLayout::eShaderReadOnlyOptimal;
        case ImageLayout::presentSrc:
            return vk::ImageLayout::ePresentSrcKHR;
        default:
            throw RendererError("Unsupported image layout");
        }
    }
};

} // namespace chronicle