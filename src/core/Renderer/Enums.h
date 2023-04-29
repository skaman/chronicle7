// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief The stage shere the shader should be executed.
enum class ShaderStage { Fragment, Vertex };

/// @brief The type used by indices.
enum class IndexType {
    undefined,

    uint16,
    uint32
};

/// @brief Alpha mode.
enum class AlphaMode {
    opaque, ///< alpha value is ignored
    mask, ///< either full opaque of fully transparent
    blend ///< output is combined with the background
};

/// @brief Data format for surface, texture or data structures.
enum class Format {
    undefined,

    // 8 bit signed int
    R8Sint,
    R8G8Sint,
    R8G8B8Sint,
    R8G8B8A8Sint,

    // 8 bit unsigned int
    R8Uint,
    R8G8Uint,
    R8G8B8Uint,
    R8G8B8A8Uint,

    // 8 bit normalized
    R8Unorm,
    R8G8Unorm,
    R8G8B8Unorm,
    R8G8B8A8Unorm,

    // 16 bit signed int
    R16Sint,
    R16G16Sint,
    R16G16B16Sint,
    R16G16B16A16Sint,

    // 16 bit unsigned int
    R16Uint,
    R16G16Uint,
    R16G16B16Uint,
    R16G16B16A16Uint,

    // 16 bit normalized
    R16Unorm,
    R16G16Unorm,
    R16G16B16Unorm,
    R16G16B16A16Unorm,

    // 32 bit signed int
    R32Sint,
    R32G32Sint,
    R32G32B32Sint,
    R32G32B32A32Sint,

    // 32 bit unsigned int
    R32Uint,
    R32G32Uint,
    R32G32B32Uint,
    R32G32B32A32Uint,

    // 32 bit signed float
    R32Sfloat,
    R32G32Sfloat,
    R32G32B32Sfloat,
    R32G32B32A32Sfloat,

    // TODO: other, to remove?
    B8G8R8A8Unorm,

    D32Sfloat,
    D32SfloatS8Uint,

    D24UnormS8Uint
};

} // namespace chronicle