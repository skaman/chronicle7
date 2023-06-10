// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

/// @brief Error exception for invalid buffer operations.
class BufferError : public GraphicsError
{
  public:
    explicit BufferError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Description for buffer creation.
struct BufferDescriptor
{
    /// @brief Buffer name.
    std::string name{};

    /// @brief The allowed usage for the buffer (required).
    BufferUsageFlags usage{};

    /// @brief The size of the buffer in bytes (required).
    uint64_t size{};

    /// @brief If true creates the buffer in an already mapped state, allowing mappedRange() to be called immediately.
    ///        It is invalid to set mappedAtCreation to true if usage does not contain eMapRead or eMapWrite.
    bool mappedAtCreation{false};
};

/// @brief A Buffer represents a block of memory that can be used in GPU operations. Data is stored in linear layout,
///        meaning that each byte of the allocation can be addressed by its offset from the start of the Buffer,
///        subject to alignment restrictions depending on the operation. Some buffers can be mapped which makes the
///        block of memory accessible via an std::span called its mapping.
class Buffer
{
  public:
    /// @brief Constructor.
    /// @param bufferDescriptor Buffer descriptor.
    explicit Buffer(const BufferDescriptor &bufferDescriptor) : _bufferDescriptor(bufferDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~Buffer() = default;

    /// @brief Maps the given range of the Buffer to be accessed with mappedRange().
    virtual void map() = 0;

    /// @brief Unmaps the mapped range of the Buffer.
    virtual void unmap() = 0;

    /// @brief Returns an std::span with the contents of the Buffer.
    /// @return Data span.
    [[nodiscard]] virtual std::span<uint8_t> mappedRange() const = 0;

    /// @brief Returns an std::span with the contents of the Buffer in the given mapped range.
    /// @param offset Offset in bytes into the buffer to return buffer contents from.
    /// @return Data span.
    [[nodiscard]] virtual std::span<uint8_t> mappedRange(uint64_t offset) const = 0;

    /// @brief Returns an std::span with the contents of the Buffer in the given mapped range.
    /// @param offset Offset in bytes into the buffer to return buffer contents from.
    /// @param size Size in bytes of the std::span to return.
    /// @return Data span.
    [[nodiscard]] virtual std::span<uint8_t> mappedRange(uint64_t offset, uint64_t size) const = 0;

    /// @brief Returns true if a buffer is mapped and getMappedRange() may be used.
    /// @return Is mapped.
    [[nodiscard]] virtual bool isMapped() const = 0;

    /// @brief Returns the descriptor used to create the buffer.
    /// @return Buffer descriptor.
    const BufferDescriptor &descriptor() const
    {
        return _bufferDescriptor;
    }

  private:
    BufferDescriptor _bufferDescriptor; ///< Buffer descriptor.
};

} // namespace chronicle::graphics