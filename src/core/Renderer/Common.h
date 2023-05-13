// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Enums.h"

namespace chronicle {

using RenderPassId = const void*;
using SamplerId = const void*;
using TextureId = const void*;

template <class T> class CommandBufferI;
template <class T> class DescriptorSetI;
template <class T> class IndexBufferI;
template <class T> class PipelineI;
template <class T> class RendererI;
template <class T> class RenderPassI;
template <class T> class ShaderCompilerI;
template <class T> class ShaderI;
template <class T> class TextureI;
template <class T> class VertexBufferI;
template <class T> class VertexBuffersI;

#ifdef VULKAN_RENDERER
class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanIndexBuffer;
class VulkanPipeline;
class VulkanRenderer;
class VulkanRenderPass;
class VulkanShaderCompiler;
class VulkanShader;
class VulkanTexture;
class VulkanVertexBuffer;
class VulkanVertexBuffers;

using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
using DescriptorSet = DescriptorSetI<VulkanDescriptorSet>;
using IndexBuffer = IndexBufferI<VulkanIndexBuffer>;
using Pipeline = PipelineI<VulkanPipeline>;
using Renderer = RendererI<VulkanRenderer>;
using RenderPass = RenderPassI<VulkanRenderPass>;
using ShaderCompiler = ShaderCompilerI<VulkanShaderCompiler>;
using Shader = ShaderI<VulkanShader>;
using Texture = TextureI<VulkanTexture>;
using VertexBuffer = VertexBufferI<VulkanVertexBuffer>;
using VertexBuffers = VertexBuffersI<VulkanVertexBuffers>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using IndexBufferRef = std::shared_ptr<IndexBuffer>;
using PipelineRef = std::shared_ptr<Pipeline>;
using RenderPassRef = std::shared_ptr<RenderPass>;
using ShaderCompilerRef = std::shared_ptr<ShaderCompiler>;
using ShaderRef = std::shared_ptr<Shader>;
using TextureRef = std::shared_ptr<Texture>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;
using VertexBuffersRef = std::shared_ptr<VertexBuffers>;

} // namespace chronicle