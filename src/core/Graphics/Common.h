// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <stdexcept>

namespace chronicle::graphics
{

class GraphicsError : public std::runtime_error
{
  public:
    using runtime_error::runtime_error;

    explicit GraphicsError(const std::string &message) : runtime_error(message.c_str())
    {
    }
};

} // namespace chronicle::platform