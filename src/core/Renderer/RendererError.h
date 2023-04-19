// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief Error that can be thrown by the renderer.
class RendererError : public std::runtime_error {
public:
    using runtime_error::runtime_error;

    /// @brief Construtor.
    /// @param message Error message.
    explicit RendererError(const std::string& message)
        : runtime_error(message.c_str())
    {
    }
};

} // namespace chronicle