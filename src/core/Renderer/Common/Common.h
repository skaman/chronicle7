// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Enums.h"

namespace chronicle {

using CommandBufferId = vk::CommandBuffer;
using DescriptorSetId = vk::DescriptorSet;
using FrameBufferId = vk::Framebuffer;
using IndexBufferId = vk::Buffer;
using PipelineId = vk::Pipeline;
using PipelineLayoutId = vk::PipelineLayout;
using RenderPassId = vk::RenderPass;
using SamplerId = vk::Sampler;
using TextureId = vk::ImageView;
using VertexBufferId = vk::Buffer;

template <class T> class BaseCommandBuffer;
template <class T> class BaseDescriptorSet;
template <class T> class BaseFrameBuffer;
template <class T> class BaseIndexBuffer;
template <class T> class BasePipeline;
template <class T> class BaseRenderContext;
template <class T> class BaseRenderPass;
template <class T> class BaseShader;
template <class T> class BaseTexture;
template <class T> class BaseVertexBuffer;

#ifdef VULKAN_RENDERER
namespace internal::vulkan {
    class VulkanCommandBuffer;
    class VulkanDescriptorSet;
    class VulkanFrameBuffer;
    class VulkanIndexBuffer;
    class VulkanPipeline;
    class VulkanRenderContext;
    class VulkanRenderPass;
    class VulkanShader;
    class VulkanTexture;
    class VulkanVertexBuffer;
} // namespace internal::vulkan

using CommandBuffer = BaseCommandBuffer<internal::vulkan::VulkanCommandBuffer>;
using DescriptorSet = BaseDescriptorSet<internal::vulkan::VulkanDescriptorSet>;
using FrameBuffer = BaseFrameBuffer<internal::vulkan::VulkanFrameBuffer>;
using IndexBuffer = BaseIndexBuffer<internal::vulkan::VulkanIndexBuffer>;
using Pipeline = BasePipeline<internal::vulkan::VulkanPipeline>;
using RenderContext = BaseRenderContext<internal::vulkan::VulkanRenderContext>;
using RenderPass = BaseRenderPass<internal::vulkan::VulkanRenderPass>;
using Shader = BaseShader<internal::vulkan::VulkanShader>;
using Texture = BaseTexture<internal::vulkan::VulkanTexture>;
using VertexBuffer = BaseVertexBuffer<internal::vulkan::VulkanVertexBuffer>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using FrameBufferRef = std::shared_ptr<FrameBuffer>;
using IndexBufferRef = std::shared_ptr<IndexBuffer>;
using PipelineRef = std::shared_ptr<Pipeline>;
using RenderPassRef = std::shared_ptr<RenderPass>;
using ShaderRef = std::shared_ptr<Shader>;
using TextureRef = std::shared_ptr<Texture>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;

} // namespace chronicle