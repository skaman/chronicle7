// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief The stage shere the shader should be executed
enum class ShaderStage { Fragment, Vertex };

/// @brief Data format for surface, texture or data structures
enum class Format {
    Undefined,
    B8G8R8A8Unorm,
    R32G32Sfloat,
    R32G32B32Sfloat,
    D32Sfloat,
    D32SfloatS8Uint,
    D24UnormS8Uint
};

} // namespace chronicle