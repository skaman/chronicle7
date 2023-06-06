// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <magic_enum.hpp>

namespace chronicle::graphics
{

enum class BufferUsageFlags : std::uint32_t
{
    eMapRead = 1 << 0,
    eMapWrite = 1 << 1,
    eCopySrc = 1 << 2,
    eCopyDst = 1 << 3,
    eIndex = 1 << 4,
    eVertex = 1 << 5,
    eUniform = 1 << 6
};

struct BufferCreateInfo
{
    std::string name{};
    BufferUsageFlags bufferUsage{};
    uint64_t size{};
};

class Buffer
{
  public:
    virtual ~Buffer() = default;

    [[nodiscard]] virtual std::span<uint8_t> data() const = 0;
};

} // namespace chronicle::graphics

template <> struct magic_enum::customize::enum_range<chronicle::graphics::BufferUsageFlags>
{
    static constexpr bool is_flags = true;
};