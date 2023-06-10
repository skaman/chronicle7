// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

#include "BindGroupLayout.h"
#include "Buffer.h"
#include "Sampler.h"
#include "TextureView.h"

#include <variant>

namespace chronicle::graphics
{

/// @brief Error exception for invalid buffer operations.
class BindGroupError : public GraphicsError
{
  public:
    explicit BindGroupError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Describes a buffer and optional range to bind as a resource.
struct BufferBinding
{
    /// @brief The buffer to bind.
    std::shared_ptr<Buffer> buffer{};

    /// @brief The offset, in bytes, from the beginning of buffer to the beginning of the range exposed to the shader
    ///        by the buffer binding.
    uint64_t offset{};

    /// @brief The size, in bytes, of the buffer binding. If not provided, specifies the range starting at offset and
    //         ending at the end of buffer.
    uint64_t size{};
};

/// @brief Describes a single resource to be bound in a BindGroup.
struct BindGroupEntry
{
    /// @brief A unique identifier for a resource binding within the BindGroup.
    uint32_t binding{};

    /// @brief The resource to bind, which may be a BufferBinding, Sampler or TextureView.
    std::variant<BufferBinding, std::shared_ptr<Sampler>, std::shared_ptr<TextureView>> resource{};
};

/// @brief Description for bind group creation.
struct BindGroupDescriptor
{
    /// @brief Bind group name.
    std::string name{};

    /// @brief The BindGroupLayout the entries of this bind group will conform to.
    std::shared_ptr<BindGroupLayout> layout{};

    /// @brief A list of entries describing the resources to expose to the shader for each binding described by the
    ///        layout.
    std::vector<BindGroupEntry> entries{};
};

/// @brief A BindGroup defines a set of resources to be bound together in a group and how the resources are used in
///        shader stages.
class BindGroup
{
  public:
    /// @brief Constructor.
    /// @param bindGroupDescriptor Bind group descriptor.
    explicit BindGroup(const BindGroupDescriptor &bindGroupDescriptor) : _bindGroupDescriptor(bindGroupDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~BindGroup() = default;

    /// @brief Returns the descriptor used to create the bind group.
    /// @return Bind group descriptor.
    const BindGroupDescriptor &descriptor() const
    {
        return _bindGroupDescriptor;
    }

  private:
    BindGroupDescriptor _bindGroupDescriptor; ///< Bind group descriptor.
};

} // namespace chronicle::graphics