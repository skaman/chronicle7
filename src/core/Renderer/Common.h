// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Enums.h"

namespace chronicle {

/// @brief Structure specifying a descriptor set layout binding.
struct DescriptorSetLayoutBinding {
    /// @brief The binding number of this entry and corresponds to a resource of the same binding number in the shader
    ///        stages.
    uint32_t binding = 0;

    /// @brief Specify the binding number of this entry and corresponds to a resource of the same binding number in the
    ///        shader stages.
    DescriptorType descriptorType = DescriptorType::unknown;

    /// @brief The number of descriptors contained in the binding, accessed in a shader as an array.
    uint32_t descriptorCount = 0;

    /// @brief Specify which pipeline shader stages can access a resource for this binding.
    ShaderStage stageFlags = ShaderStage::none;
};

/// @brief Data structure that contain descriptor set informations read from the shader itself with spirv-reflect.
struct DescriptorSetLayout {
    uint32_t setNumber = 0; ///< Set number.
    std::vector<DescriptorSetLayoutBinding> bindings = {}; ///< Descriptor set layout bindings
};

template <class T> class CommandBufferI;
template <class T> class DescriptorSetI;
template <class T> class VertexBufferI;
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
class VulkanPipeline;
class VulkanIndexBuffer;
class VulkanTexture;
class VulkanRenderer;
class VulkanMesh;
class VulkanMaterial;
class VulkanShader;
class VulkanShaderCompiler;

using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
using DescriptorSet = DescriptorSetI<VulkanDescriptorSet>;
using VertexBuffer = VertexBufferI<VulkanVertexBuffer>;
using Pipeline = PipelineI<VulkanPipeline>;
using IndexBuffer = IndexBufferI<VulkanIndexBuffer>;
using Texture = TextureI<VulkanTexture>;
using Renderer = RendererI<VulkanRenderer>;
using Mesh = MeshI<VulkanMesh>;
using Material = MaterialI<VulkanMaterial>;
using Shader = ShaderI<VulkanShader>;
using ShaderCompiler = ShaderCompilerI<VulkanShaderCompiler>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;
using PipelineRef = std::shared_ptr<Pipeline>;
using IndexBufferRef = std::shared_ptr<IndexBuffer>;
using TextureRef = std::shared_ptr<Texture>;
using MeshRef = std::shared_ptr<Mesh>;
using MaterialRef = std::shared_ptr<Material>;
using ShaderRef = std::shared_ptr<Shader>;
using ShaderCompilerRef = std::shared_ptr<ShaderCompiler>;

} // namespace chronicle