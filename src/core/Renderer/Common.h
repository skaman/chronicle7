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

template <class T> class CommandBufferI;
template <class T> class DescriptorSetI;
template <class T> class IndexBufferI;
template <class T> class FrameBufferI;
template <class T> class PipelineI;
template <class T> class RenderContextI;
template <class T> class RenderPassI;
template <class T> class ShaderCompilerI;
template <class T> class ShaderI;
template <class T> class TextureI;
template <class T> class VertexBufferI;

#ifdef VULKAN_RENDERER
class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanIndexBuffer;
class VulkanFrameBuffer;
class VulkanPipeline;
class VulkanRenderContext;
class VulkanRenderPass;
class VulkanShaderCompiler;
class VulkanShader;
class VulkanTexture;
class VulkanVertexBuffer;

using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
using DescriptorSet = DescriptorSetI<VulkanDescriptorSet>;
using IndexBuffer = IndexBufferI<VulkanIndexBuffer>;
using FrameBuffer = FrameBufferI<VulkanFrameBuffer>;
using Pipeline = PipelineI<VulkanPipeline>;
using RenderContext = RenderContextI<VulkanRenderContext>;
using RenderPass = RenderPassI<VulkanRenderPass>;
using ShaderCompiler = ShaderCompilerI<VulkanShaderCompiler>;
using Shader = ShaderI<VulkanShader>;
using Texture = TextureI<VulkanTexture>;
using VertexBuffer = VertexBufferI<VulkanVertexBuffer>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using IndexBufferRef = std::shared_ptr<IndexBuffer>;
using FrameBufferRef = std::shared_ptr<FrameBuffer>;
using PipelineRef = std::shared_ptr<Pipeline>;
using RenderPassRef = std::shared_ptr<RenderPass>;
using ShaderCompilerRef = std::shared_ptr<ShaderCompiler>;
using ShaderRef = std::shared_ptr<Shader>;
using TextureRef = std::shared_ptr<Texture>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;

} // namespace chronicle