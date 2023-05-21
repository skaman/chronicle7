// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "RenderPassInfo.h"

namespace chronicle {

/// @brief Object used to handle a render pass.
/// @tparam T Type with implementation.
template <class T> class RenderPassI {
public:
    /// @brief Get the render pass handle ID
    /// @return Render pass ID
    [[nodiscard]] RenderPassId renderPassId() const { return static_cast<const T*>(this)->renderPassId(); }

    /// @brief Get the hash of the configuration used to create the render pass.
    /// @return Render pass hash.
    [[nodiscard]] size_t hash() const { return static_cast<const T*>(this)->hash(); };

    /// @brief Get the format used by the color attachment.
    /// @return Format.
    [[nodiscard]] Format format() const { return static_cast<const T*>(this)->format(); };

    /// @brief Get the multi sampling used by the color attachment.
    /// @return Multi sampling.
    [[nodiscard]] MSAA msaa() const { return static_cast<const T*>(this)->msaa(); };

    /// @brief Factory for create a new render pass.
    /// @param renderPassInfo Informations used to create the render pass.
    /// @param name Render pass name.
    /// @return The render pass.
    [[nodiscard]] static RenderPassRef create(const RenderPassInfo& renderPassInfo, const std::string& name)
    {
        return T::create(renderPassInfo, name);
    }

private:
    RenderPassI() = default;
    friend T;
};

} // namespace chronicle