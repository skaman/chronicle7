// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"
#include "Enums.h"

namespace chronicle {

template <class T> class CommandBufferI;
template <class T> class DescriptorSetI;
template <class T> class VertexBufferI;
template <class T> class PipelineI;
template <class T> class IndexBufferI;
template <class T> class TextureI;
template <class T> class RendererI;

#ifdef VULKAN_RENDERER
class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanVertexBuffer;
class VulkanPipeline;
class VulkanIndexBuffer;
class VulkanTexture;
class VulkanRenderer;

using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
using DescriptorSet = DescriptorSetI<VulkanDescriptorSet>;
using VertexBuffer = VertexBufferI<VulkanVertexBuffer>;
using Pipeline = PipelineI<VulkanPipeline>;
using IndexBuffer = IndexBufferI<VulkanIndexBuffer>;
using Texture = TextureI<VulkanTexture>;
using Renderer = RendererI<VulkanRenderer>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;
using PipelineRef = std::shared_ptr<Pipeline>;
using IndexBufferRef = std::shared_ptr<IndexBuffer>;
using TextureRef = std::shared_ptr<Texture>;

} // namespace chronicle