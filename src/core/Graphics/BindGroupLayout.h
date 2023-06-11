// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

#include <variant>

namespace chronicle::graphics
{

/// @brief Error exception for invalid bind group layout operations.
class BindGroupLayoutError : public GraphicsError
{
  public:
    explicit BindGroupLayoutError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Informations for a buffer binding layout
struct BufferBindingLayout
{
    /// @brief Indicates the type required for buffers bound to this bindings.
    BufferBindingType type{BufferBindingType::eUniform};

    /// @brief Indicates whether this binding requires a dynamic offset.
    bool hasDynamicOffset{false};

    /// @brief Indicates the minimum size of a buffer binding used with this bind point.
    uint64_t minBindingSize{0};
};

/// @brief Informations for a sampler binding layout
struct SamplerBindingLayout
{
    /// @brief Indicates the required type of a sampler bound to this bindings.
    SamplerBindingType type{SamplerBindingType::eFiltering};
};

/// @brief Informations for a texture binding layout
struct TextureBindingLayout
{
    /// @brief Indicates the type required for texture views bound to this binding.
    TextureSampleType type{TextureSampleType::eFloat};

    /// @brief Indicates the required dimension for texture views bound to this binding.
    TextureViewDimension viewDimension{TextureViewDimension::e2D};

    /// @brief Indicates whether or not texture views bound to this binding must be multisampled.
    bool multisampled{false};
};

/// @brief Informations for a storage texture binding layout
struct StorageTextureBindingLayout
{
    /// @brief The access mode for this binding, indicating readability and writability.
    StorageTextureAccess access{StorageTextureAccess::eWriteOnly};

    /// @brief The required format of texture views bound to this binding.
    TextureFormat format{};

    /// @brief Indicates the required dimension for texture views bound to this binding.
    TextureViewDimension viewDimension{TextureViewDimension::e2D};
};

/// @brief A BindGroupLayoutEntry describes a single shader resource binding to be included in a BindGroupLayout.
struct BindGroupLayoutEntry
{
    /// @brief A unique identifier for a resource binding within the BindGroupLayout.
    uint32_t binding{};

    /// @brief A bitset of the members of ShaderStage. Each set bit indicates that a BindGroupLayoutEntry's resource
    ///        will be accessible from the associated shader stage.
    ShaderStageFlags visibility{};

    /// @brief Layout description which may be a BufferBindingLayout, SamplerBindingLayout, TextureBindingLayout or
    ///        StorageTextureBindingLayout.
    std::variant<BufferBindingLayout, SamplerBindingLayout, TextureBindingLayout, StorageTextureBindingLayout> layout{};
};

/// @brief Description for bind group layout creation.
struct BindGroupLayoutDescriptor
{
    /// @brief Bind group layout name.
    std::string name{};

    /// @brief Entries that compose the bind group layout.
    std::vector<BindGroupLayoutEntry> entries{};
};

/// @brief A BindGroupLayout defines the interface between a set of resources bound in a BindGroup and their
///        accessibility in shader stages.
class BindGroupLayout
{
  public:
    /// @brief Constructor.
    /// @param bindGroupLayoutDescriptor Bind group layout descriptor.
    explicit BindGroupLayout(const BindGroupLayoutDescriptor &bindGroupLayoutDescriptor)
        : _bindGroupLayoutDescriptor(bindGroupLayoutDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~BindGroupLayout() = default;

    /// @brief Returns the descriptor used to create the bind group layout.
    /// @return Bind group layout descriptor.
    const BindGroupLayoutDescriptor &descriptor() const
    {
        return _bindGroupLayoutDescriptor;
    }

  private:
    BindGroupLayoutDescriptor _bindGroupLayoutDescriptor; ///< Bind group layout descriptor.
};

} // namespace chronicle::graphics