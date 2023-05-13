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

    /// @brief Factory for create a new render pass.
    /// @param renderPassInfo Informations used to create the render pass.
    /// @return The render pass.
    [[nodiscard]] static RenderPassRef create(const RenderPassInfo& renderPassInfo)
    {
        return T::create(renderPassInfo);
    }

private:
    RenderPassI() = default;
    friend T;
};

} // namespace chronicle