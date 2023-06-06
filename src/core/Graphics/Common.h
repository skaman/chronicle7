// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <stdexcept>

namespace chronicle::graphics
{

/// @brief Informations used to initialize the graphic system.
struct SystemInitInfo
{
    std::string applicationName{"Chronicle"}; /// Application name.
    bool enableDebug{};                       /// Enable graphic debug informations.
};

/// @brief Informations used to request a graphic device.
struct RequestDeviceInfo
{
    void *hwnd{}; /// Window handler that the device should need to be compatible to.
};

class GraphicsError : public std::runtime_error
{
  public:
    using runtime_error::runtime_error;

    explicit GraphicsError(const std::string &message) : runtime_error(message.c_str())
    {
    }
};

} // namespace chronicle::graphics