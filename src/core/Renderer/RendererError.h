// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

class RendererError : public std::runtime_error {
public:
    using runtime_error::runtime_error;

    explicit RendererError(const std::string& message)
        : runtime_error(message.c_str())
    {
    }
};

} // namespace chronicle