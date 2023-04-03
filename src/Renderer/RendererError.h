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