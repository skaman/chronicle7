// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Enums.h"

namespace chronicle {

using TextureId = const void*;
using SamplerId = const void*;

template <class T> class CommandBufferI;
template <class T> class DescriptorSetI;
template <class T> class VertexBufferI;
template <class T> class VertexBuffersI;
template <class T> class PipelineI;
template <class T> class IndexBufferI;
template <class T> class TextureI;
template <class T> class RendererI;
template <class T> class MeshI;
template <class T> class MaterialI;
template <class T> class ShaderI;
template <class T> class ShaderCompilerI;

#ifdef VULKAN_RENDERER
class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanVertexBuffer;
class VulkanVertexBuffers;
class VulkanPipeline;
class VulkanIndexBuffer;
class VulkanTexture;
class VulkanRenderer;
class VulkanShader;
class VulkanShaderCompiler;

using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
using DescriptorSet = DescriptorSetI<VulkanDescriptorSet>;
using VertexBuffer = VertexBufferI<VulkanVertexBuffer>;
using VertexBuffers = VertexBuffersI<VulkanVertexBuffers>;
using Pipeline = PipelineI<VulkanPipeline>;
using IndexBuffer = IndexBufferI<VulkanIndexBuffer>;
using Texture = TextureI<VulkanTexture>;
using Renderer = RendererI<VulkanRenderer>;
using Shader = ShaderI<VulkanShader>;
using ShaderCompiler = ShaderCompilerI<VulkanShaderCompiler>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;
using VertexBuffersRef = std::shared_ptr<VertexBuffers>;
using PipelineRef = std::shared_ptr<Pipeline>;
using IndexBufferRef = std::shared_ptr<IndexBuffer>;
using TextureRef = std::shared_ptr<Texture>;
using ShaderRef = std::shared_ptr<Shader>;
using ShaderCompilerRef = std::shared_ptr<ShaderCompiler>;

} // namespace chronicle